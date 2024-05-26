#include "orientation_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

bool OrientationExperiment::verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	run_helper.num_actions_limit = MAX_NUM_ACTIONS_LIMIT_MULTIPLIER * solution->explore_scope_max_num_actions;

	run_helper.num_decisions++;

	vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		int curr_layer = 0;
		ScopeHistory* curr_scope_history = context.back().scope_history;
		while (true) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
				this->input_node_contexts[i_index][curr_layer]);
			if (it == curr_scope_history->node_histories.end()) {
				break;
			} else {
				if (curr_layer == (int)this->input_scope_contexts[i_index].size()-1) {
					switch (it->first->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							if (branch_node_history->is_branch) {
								input_vals[i_index] = 1.0;
							} else {
								input_vals[i_index] = -1.0;
							}
						}
						break;
					case NODE_TYPE_INFO_SCOPE:
						{
							InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
							if (info_scope_node_history->is_positive) {
								input_vals[i_index] = 1.0;
							} else {
								input_vals[i_index] = -1.0;
							}
						}
						break;
					case NODE_TYPE_INFO_BRANCH:
						{
							InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
							if (info_branch_node_history->is_branch) {
								input_vals[i_index] = 1.0;
							} else {
								input_vals[i_index] = -1.0;
							}
						}
						break;
					}
					break;
				} else {
					curr_layer++;
					curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
				}
			}
		}
	}

	double existing_predicted_score = this->existing_average_score;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		existing_predicted_score += input_vals[i_index] * this->existing_linear_weights[i_index];
	}
	if (this->existing_network != NULL) {
		vector<vector<double>> existing_network_input_vals(this->existing_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
			existing_network_input_vals[i_index] = vector<double>(this->existing_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->existing_network_input_indexes[i_index].size(); s_index++) {
				existing_network_input_vals[i_index][s_index] = input_vals[this->existing_network_input_indexes[i_index][s_index]];
			}
		}
		this->existing_network->activate(existing_network_input_vals);
		existing_predicted_score += this->existing_network->output->acti_vals[0];
	}

	double new_predicted_score = this->new_average_score;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		new_predicted_score += input_vals[i_index] * this->new_linear_weights[i_index];
	}
	if (this->new_network != NULL) {
		vector<vector<double>> new_network_input_vals(this->new_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
			new_network_input_vals[i_index] = vector<double>(this->new_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->new_network_input_indexes[i_index].size(); s_index++) {
				new_network_input_vals[i_index][s_index] = input_vals[this->new_network_input_indexes[i_index][s_index]];
			}
		}
		this->new_network->activate(new_network_input_vals);
		new_predicted_score += this->new_network->output->acti_vals[0];
	}

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch = new_predicted_score >= existing_predicted_score;
	#endif /* MDEBUG */

	if (decision_is_branch) {
		if (this->step_types.size() == 0) {
			curr_node = this->exit_next_node;
		} else {
			if (this->step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->actions[0];
			} else {
				curr_node = this->scopes[0];
			}
		}

		return true;
	} else {
		return false;
	}
}

void OrientationExperiment::verify_backprop(
		EvalHistory* outer_eval_history,
		EvalHistory* eval_history,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (run_helper.num_actions_limit == 0) {
		run_helper.num_actions_limit = -1;

		this->result = EXPERIMENT_RESULT_FAIL;
	} else {
		double target_impact;
		if (context.size() == 1) {
			target_impact = problem->score_result(run_helper.num_decisions);
		} else {
			target_impact = context[context.size()-2].scope->eval->calc_impact(outer_eval_history);
		}

		double combined_predicted_impact = this->eval_context->calc_impact(eval_history);

		double combined_misguess = (target_impact - combined_predicted_impact) * (target_impact - combined_predicted_impact);
		this->combined_score += combined_misguess;

		run_helper.num_actions_limit = -1;

		this->state_iter++;
		if (this->state == ORIENTATION_EXPERIMENT_STATE_VERIFY_1ST
				&& this->state_iter >= VERIFY_1ST_NUM_DATAPOINTS) {
			this->combined_score /= VERIFY_1ST_NUM_DATAPOINTS;

			#if defined(MDEBUG) && MDEBUG
			// if (rand()%2 == 0) {
			if (true) {
			#else
			if (this->combined_score <= this->verify_existing_average_score) {
			#endif /* MDEBUG */
				this->target_val_histories.reserve(VERIFY_2ND_NUM_DATAPOINTS);

				this->state = ORIENTATION_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		} else if (this->state_iter >= VERIFY_2ND_NUM_DATAPOINTS) {
			this->combined_score /= VERIFY_2ND_NUM_DATAPOINTS;

			#if defined(MDEBUG) && MDEBUG
			// if (rand()%2 == 0) {
			if (true) {
			#else
			if (this->combined_score < this->verify_existing_average_score) {
			#endif /* MDEBUG */
				cout << "OrientationExperiment" << endl;
				cout << "verify" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
					if (this->step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->actions[s_index]->action.move;
					} else {
						cout << " E";
					}
				}
				cout << endl;

				if (this->exit_next_node == NULL) {
					cout << "this->exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
				}

				cout << "this->combined_score: " << this->combined_score << endl;
				cout << "this->verify_existing_average_score: " << this->verify_existing_average_score << endl;

				cout << "this->branch_weight: " << this->branch_weight << endl;

				cout << endl;

				#if defined(MDEBUG) && MDEBUG
				if (this->is_pass_through) {
					this->result = EXPERIMENT_RESULT_SUCCESS;
				} else {
					this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
					this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

					this->state = ORIENTATION_EXPERIMENT_STATE_CAPTURE_VERIFY;
					this->state_iter = 0;
				}
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
