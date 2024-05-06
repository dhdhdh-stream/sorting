#include "info_pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "network.h"
#include "scope.h"
#include "utilities.h"

using namespace std;

void InfoPassThroughExperiment::verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		InfoPassThroughExperimentHistory* history) {
	if (this->info_scope == NULL) {
		if (this->step_types.size() == 0) {
			curr_node = this->exit_next_node;
		} else {
			if (this->step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->actions[0];
			} else {
				curr_node = this->scopes[0];
			}
		}
	} else {
		ScopeHistory* inner_scope_history;
		bool inner_is_positive;
		this->info_scope->activate(problem,
								   run_helper,
								   inner_scope_history,
								   inner_is_positive);

		delete inner_scope_history;

		if ((this->is_negate && !inner_is_positive)
				|| (!this->is_negate && inner_is_positive)) {
			if (this->step_types.size() == 0) {
				curr_node = this->exit_next_node;
			} else {
				if (this->step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->actions[0];
				} else {
					curr_node = this->scopes[0];
				}
			}
		}
	}

	context.back().scope_history->info_experiment_history = history;
}

void InfoPassThroughExperiment::verify_back_activate(
		ScopeHistory*& subscope_history,
		bool& result_is_positive,
		RunHelper& run_helper) {
	if (this->new_state == INFO_SCOPE_STATE_DISABLED_NEGATIVE) {
		result_is_positive = false;
	} else if (this->new_state == INFO_SCOPE_STATE_DISABLED_POSITIVE) {
		result_is_positive = true;
	} else {
		vector<double> input_vals(this->input_node_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = subscope_history->node_histories.find(
				this->input_node_contexts[i_index]);
			if (it != subscope_history->node_histories.end()) {
				switch (this->input_node_contexts[i_index]->type) {
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
			}
		}

		double negative_score = this->negative_average_score;
		for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
			negative_score += input_vals[i_index] * this->negative_linear_weights[i_index];
		}
		if (this->negative_network != NULL) {
			vector<vector<double>> negative_network_input_vals(this->negative_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->negative_network_input_indexes.size(); i_index++) {
				negative_network_input_vals[i_index] = vector<double>(this->negative_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->negative_network_input_indexes[i_index].size(); v_index++) {
					negative_network_input_vals[i_index][v_index] = input_vals[this->negative_network_input_indexes[i_index][v_index]];
				}
			}
			this->negative_network->activate(negative_network_input_vals);
			negative_score += this->negative_network->output->acti_vals[0];
		}

		double positive_score = this->positive_average_score;
		for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
			positive_score += input_vals[i_index] * this->positive_linear_weights[i_index];
		}
		if (this->positive_network != NULL) {
			vector<vector<double>> positive_network_input_vals(this->positive_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->positive_network_input_indexes.size(); i_index++) {
				positive_network_input_vals[i_index] = vector<double>(this->positive_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->positive_network_input_indexes[i_index].size(); v_index++) {
					positive_network_input_vals[i_index][v_index] = input_vals[this->positive_network_input_indexes[i_index][v_index]];
				}
			}
			this->positive_network->activate(positive_network_input_vals);
			positive_score += this->positive_network->output->acti_vals[0];
		}

		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			result_is_positive = true;
		} else {
			result_is_positive = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		if (positive_score >= negative_score) {
			result_is_positive = true;
		} else {
			result_is_positive = false;
		}
		#endif /* MDEBUG */
	}
}

void InfoPassThroughExperiment::verify_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (this->state == INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
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

		if (new_average_score > this->existing_average_score) {
		#endif /* MDEBUG */
			this->o_target_val_histories.reserve(VERIFY_2ND_NUM_DATAPOINTS);

			this->state = INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
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
			cout << "InfoPassThrough" << endl;
			cout << "this->info_scope_context->id: " << this->info_scope_context->id << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			if (this->info_scope != NULL) {
				cout << "this->info_scope->id: " << this->info_scope->id << endl;
			}
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->actions[s_index]->action.move;
				} else if (this->step_types[s_index] == STEP_TYPE_SCOPE) {
					cout << " E";
				}
			}
			cout << endl;

			if (this->exit_next_node == NULL) {
				cout << "this->exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
			}

			cout << "this->existing_average_score: " << this->existing_average_score << endl;
			cout << "new_average_score: " << new_average_score << endl;

			#if defined(MDEBUG) && MDEBUG
			if (this->new_state == INFO_SCOPE_STATE_NA) {
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = INFO_PASS_THROUGH_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_SUCCESS;
			}
			#else
			this->result = EXPERIMENT_RESULT_SUCCESS;
			#endif /* MDEBUG */
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
