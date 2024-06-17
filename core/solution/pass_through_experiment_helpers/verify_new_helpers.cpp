#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_scope.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void PassThroughExperiment::verify_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		PassThroughExperimentHistory* history) {
	history->predicted_scores.push_back(vector<double>(context.size()-1, 0.0));
	for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
		Scope* scope = (Scope*)context[l_index].scope;
		ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;
		if (scope->eval_network != NULL) {
			scope_history->callback_experiment_history = history;
			scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			scope_history->callback_experiment_layers.push_back(l_index);
		}
	}

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
		double inner_score;
		this->best_info_scope->activate(problem,
										run_helper,
										inner_score);

		bool is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		if (this->best_is_negate) {
			if (inner_score >= 0.0) {
				is_branch = false;
			} else {
				is_branch = true;
			}
		} else {
			if (inner_score >= 0.0) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}
		#endif /* MDEBUG */

		if (is_branch) {
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

void PassThroughExperiment::verify_new_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(scope_history);
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void PassThroughExperiment::verify_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		double sum_score = 0.0;
		for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
			sum_score += history->predicted_scores[i_index][l_index];
		}
		sum_score += target_val - solution->average_score;
		double final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);
		this->target_val_histories.push_back(final_score);
	}

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= VERIFY_NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		double new_average_score = sum_scores / num_instances;

		this->target_val_histories.clear();

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

				this->root_experiment->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

				this->root_experiment->root_state = ROOT_EXPERIMENT_STATE_VERIFY_EXISTING;

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
