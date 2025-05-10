#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 5;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 10;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 40;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 400;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 4000;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 100;
#endif /* MDEBUG */

void PassThroughExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			problem->perform_action(this->actions[s_index]);
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[s_index]);
			this->scopes[s_index]->activate(problem,
				run_helper,
				inner_scope_history);
			delete inner_scope_history;
		}

		run_helper.num_actions += 2;
	}

	curr_node = this->exit_next_node;
}

void PassThroughExperiment::explore_backprop(
		double target_val,
		bool is_return,
		RunHelper& run_helper) {
	this->sum_score += target_val;

	this->state_iter++;
	bool is_fail = false;
	if (!is_return) {
		is_fail = true;
	} else {
		switch (this->state) {
		case PASS_THROUGH_EXPERIMENT_STATE_INITIAL:
			if (this->state_iter == INITIAL_NUM_SAMPLES_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (this->allow_bad || rand()%2 == 0) {
				#else
				double curr_score = this->sum_score / this->state_iter;
				if (this->allow_bad || curr_score <= this->existing_average_score) {
				#endif /* MDEBUG */
					is_fail = true;
				} else {
					this->sum_score = 0.0;

					this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST;
					this->state_iter = 0;
				}
			}
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
			if (this->state_iter == VERIFY_1ST_NUM_SAMPLES_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (this->allow_bad || rand()%2 == 0) {
				#else
				double curr_score = this->sum_score / this->state_iter;
				if (this->allow_bad || curr_score <= this->existing_average_score) {
				#endif /* MDEBUG */
					is_fail = true;
				} else {
					this->sum_score = 0.0;

					this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND;
					this->state_iter = 0;
				}
			}
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
			if (this->state_iter == VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
				double curr_score = this->sum_score / this->state_iter;
				#if defined(MDEBUG) && MDEBUG
				if (this->allow_bad || rand()%2 == 0) {
				#else
				if (this->allow_bad || curr_score <= this->existing_average_score) {
				#endif /* MDEBUG */
					is_fail = true;
				} else {
					this->true_improvement = curr_score - this->existing_average_score;

					cout << "PassThrough" << endl;
					cout << "this->scope_context->id: " << this->scope_context->id << endl;
					cout << "this->node_context->id: " << this->node_context->id << endl;
					cout << "this->is_branch: " << this->is_branch << endl;
					cout << "new explore path:";
					for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
						if (this->step_types[s_index] == STEP_TYPE_ACTION) {
							cout << " " << this->actions[s_index].move;
						} else {
							cout << " E" << this->scopes[s_index]->id;
						}
					}
					cout << endl;

					cout << "this->true_improvement: " << this->true_improvement << endl;

					this->result = EXPERIMENT_RESULT_SUCCESS;
				}
			}
			break;
		}
	}

	if (is_fail) {
		this->explore_iter++;
		if (this->explore_iter >= PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS) {
			this->result = EXPERIMENT_RESULT_FAIL;
		} else {
			this->step_types.clear();
			this->actions.clear();
			this->scopes.clear();

			geometric_distribution<int> geo_distribution(0.2);
			int new_num_steps = geo_distribution(generator);

			/**
			 * - always give raw actions a large weight
			 *   - existing scopes often learned to avoid certain patterns
			 *     - which can prevent innovation
			 */
			uniform_int_distribution<int> scope_distribution(0, 1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				if (scope_distribution(generator) == 0 && this->scope_context->child_scopes.size() > 0) {
					this->step_types.push_back(STEP_TYPE_SCOPE);
					this->actions.push_back(Action());

					uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
					this->scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
				} else {
					this->step_types.push_back(STEP_TYPE_ACTION);

					this->actions.push_back(problem_type->random_action());

					this->scopes.push_back(NULL);
				}
			}

			this->sum_score = 0.0;

			this->state = PASS_THROUGH_EXPERIMENT_STATE_INITIAL;
			this->state_iter = 0;
		}
	}
}
