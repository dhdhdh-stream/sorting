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
const int INITIAL_NUM_SAMPLES_PER_ITER = 100;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 500;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 2000;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 100;
#endif /* MDEBUG */

void PassThroughExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper) {
	run_helper.has_explore = true;

	if (this->state_iter == -1) {
		vector<AbstractNode*> possible_exits;

		AbstractNode* starting_node;
		switch (this->node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				starting_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;
				starting_node = obs_node->next_node;
			}
			break;
		}

		this->scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		int random_index;
		if (this->scope_context->exceeded) {
			if (possible_exits.size() <= 4) {
				this->result = EXPERIMENT_RESULT_FAIL;
				return;
			} else {
				uniform_int_distribution<int> distribution(4, possible_exits.size()-1);
				random_index = distribution(generator);
			}
		} else {
			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			random_index = distribution(generator);
		}
		this->curr_exit_next_node = possible_exits[random_index];

		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.2);
		if (random_index == 0) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}
		if (this->scope_context->exceeded) {
			if (new_num_steps > random_index/2-1) {
				new_num_steps = random_index/2-1;
			}
		}

		/**
		 * - always give raw actions a large weight
		 *   - existing scopes often learned to avoid certain patterns
		 *     - which can prevent innovation
		 */
		uniform_int_distribution<int> scope_distribution(0, 1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (scope_distribution(generator) == 0 && this->scope_context->child_scopes.size() > 0) {
				this->curr_step_types.push_back(STEP_TYPE_SCOPE);
				this->curr_actions.push_back(Action());

				uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
				this->curr_scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
			} else {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				this->curr_actions.push_back(problem_type->random_action());

				this->curr_scopes.push_back(NULL);
			}
		}

		this->state_iter = 0;
	}

	for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
		if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
			problem->perform_action(this->curr_actions[s_index]);
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[s_index]);
			this->curr_scopes[s_index]->activate(problem,
				run_helper,
				inner_scope_history);
			delete inner_scope_history;
		}

		run_helper.num_actions += 2;
	}

	curr_node = this->curr_exit_next_node;
}

void PassThroughExperiment::explore_backprop(
		double target_val,
		RunHelper& run_helper) {
	bool is_fail = false;

	this->curr_sum_score += target_val;

	this->state_iter++;
	if (this->state_iter == INITIAL_NUM_SAMPLES_PER_ITER
			|| this->state_iter == VERIFY_1ST_NUM_SAMPLES_PER_ITER
			|| this->state_iter == VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
		#if defined(MDEBUG) && MDEBUG
		if (false) {
		#else
		double curr_score = this->curr_sum_score / this->state_iter;
		double existing_score = this->node_context->sum_score / this->node_context->num_measure;
		if (curr_score <= existing_score) {
		#endif /* MDEBUG */
			is_fail = true;
		}
	}

	bool is_next = false;
	if (is_fail) {
		this->curr_sum_score = 0.0;
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();

		is_next = true;
	} else if (this->state_iter >= VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
		double curr_score = this->curr_sum_score / this->state_iter;
		this->best_score = curr_score;
		this->best_step_types = this->curr_step_types;
		this->best_actions = this->curr_actions;
		this->best_scopes = this->curr_scopes;
		this->best_exit_next_node = this->curr_exit_next_node;

		this->curr_sum_score = 0.0;
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();

		is_next = true;
	}

	if (is_next) {
		this->explore_iter++;
		if (this->explore_iter >= PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (this->best_score != numeric_limits<double>::lowest()) {
			#else
			double existing_score = this->node_context->sum_score / this->node_context->num_measure;
			if (this->best_score > existing_score) {
			#endif /* MDEBUG */
				cout << "PassThrough" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index].move;
					} else {
						cout << " E" << this->best_scopes[s_index]->id;
					}
				}
				cout << endl;

				cout << "this->best_score: " << this->best_score << endl;

				this->result = EXPERIMENT_RESULT_SUCCESS;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		} else {
			this->state_iter = -1;
		}
	}
}
