#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "info_scope.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::verify_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper) {
	if (this->best_info_scope == NULL) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}
	} else {
		ScopeHistory* inner_scope_history;
		bool inner_is_positive;
		this->best_info_scope->activate(problem,
										run_helper,
										inner_scope_history,
										inner_is_positive);

		delete inner_scope_history;

		if ((this->best_is_negate && !inner_is_positive)
				|| (!this->best_is_negate && inner_is_positive)) {
			if (this->best_step_types.size() == 0) {
				curr_node = this->best_exit_next_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else {
					curr_node = this->best_scopes[0];
				}
			}
		}
	}
}

void PassThroughExperiment::verify_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (this->state == PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW
			&& (int)this->o_target_val_histories.size() >= VERIFY_1ST_NUM_DATAPOINTS) {
		#if defined(MDEBUG) && MDEBUG
		this->o_target_val_histories.clear();

		if (rand()%2 == 0) {
		#else
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		double new_average_score = sum_scores / VERIFY_1ST_NUM_DATAPOINTS;

		this->o_target_val_histories.clear();

		if (new_average_score >= this->existing_average_score) {
		#endif /* MDEBUG */
			this->o_target_val_histories.reserve(VERIFY_2ND_NUM_DATAPOINTS);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	} else if ((int)this->o_target_val_histories.size() >= VERIFY_2ND_NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		double new_average_score = sum_scores / VERIFY_2ND_NUM_DATAPOINTS;

		this->o_target_val_histories.clear();

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (new_average_score > this->existing_average_score) {
		#endif /* MDEBUG */
			cout << "PassThrough" << endl;
			cout << "this->parent_experiment: " << this->parent_experiment << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.move;
				} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
					cout << " E";
				}
			}
			cout << endl;

			if (this->best_exit_next_node == NULL) {
				cout << "this->best_exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
			}

			cout << "this->existing_average_score: " << this->existing_average_score << endl;
			cout << "new_average_score: " << new_average_score << endl;

			if (this->parent_experiment == NULL) {
				this->result = EXPERIMENT_RESULT_SUCCESS;
			} else {
				vector<AbstractExperiment*> verify_experiments;
				AbstractExperiment* curr_experiment = this;
				while (true) {
					if (curr_experiment->parent_experiment == NULL) {
						/**
						 * - don't include root
						 */
						break;
					} else {
						verify_experiments.insert(verify_experiments.begin(), curr_experiment);
						curr_experiment = curr_experiment->parent_experiment;
					}
				}

				this->root_experiment->verify_experiments = verify_experiments;

				this->root_experiment->o_target_val_histories.reserve(VERIFY_1ST_NUM_DATAPOINTS);

				this->root_experiment->root_state = ROOT_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;

				this->state = PASS_THROUGH_EXPERIMENT_STATE_ROOT_VERIFY;
			}
		} else {
			if (this->best_step_types.size() > 0) {
				this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
				this->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
				this->experiment_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
