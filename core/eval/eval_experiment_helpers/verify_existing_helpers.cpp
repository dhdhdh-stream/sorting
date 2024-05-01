#include "eval_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void EvalExperiment::verify_existing_activate(
		vector<ContextLayer>& context,
		EvalExperimentHistory* history) {
	context.back().scope_history->experiment_history = history;
}

void EvalExperiment::verify_existing_back_activate(
		vector<ContextLayer>& context,
		EvalExperimentHistory* history) {
	vector<double> input_vals(solution->eval->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)solution->eval->input_node_contexts.size(); i_index++) {
		int curr_layer = 0;
		ScopeHistory* curr_scope_history = context.back().scope_history;
		while (true) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
				solution->eval->input_node_contexts[i_index][curr_layer]);
			if (it == curr_scope_history->node_histories.end()) {
				break;
			} else {
				if (curr_layer == (int)solution->eval->input_node_contexts[i_index].size()-1) {
					if (it->first->type == NODE_TYPE_ACTION) {
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						input_vals[i_index] = action_node_history->obs_snapshot[solution->eval->input_obs_indexes[i_index]];
					} else {
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							input_vals[i_index] = 1.0;
						} else {
							input_vals[i_index] = -1.0;
						}
					}
					break;
				} else {
					curr_layer++;
					curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
				}
			}
		}
	}

	double score = solution->eval->average_score;
	for (int i_index = 0; i_index < (int)solution->eval->linear_input_indexes.size(); i_index++) {
		score += input_vals[solution->eval->linear_input_indexes[i_index]] * solution->eval->linear_weights[i_index];
	}
	if (solution->eval->network != NULL) {
		vector<vector<double>> network_input_vals(solution->eval->network_input_indexes.size());
		for (int i_index = 0; i_index < (int)solution->eval->network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(solution->eval->network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)solution->eval->network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = input_vals[solution->eval->network_input_indexes[i_index][v_index]];
			}
		}
		solution->eval->network->activate(network_input_vals);
		score += solution->eval->network->output->acti_vals[0];
	}

	history->predicted_scores.push_back(score);
}

void EvalExperiment::verify_existing_backprop(double target_val,
											  RunHelper& run_helper) {
	EvalExperimentHistory* history = (EvalExperimentHistory*)run_helper.experiment_histories.back();

	for (int h_index = 0; h_index < (int)history->predicted_scores.size(); h_index++) {
		this->original_average_misguess += (history->predicted_scores[h_index] - target_val) * (history->predicted_scores[h_index] - target_val);

		this->state_iter++;
	}

	if (this->state == EVAL_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
			&& this->state_iter >= VERIFY_1ST_NUM_DATAPOINTS) {
		this->original_average_misguess /= this->state_iter;

		this->combined_misguess = 0.0;

		this->state = EVAL_EXPERIMENT_STATE_VERIFY_1ST;
		this->state_iter = 0;
	} else if (this->state_iter >= VERIFY_2ND_NUM_DATAPOINTS) {
		this->original_average_misguess /= this->state_iter;

		this->combined_misguess = 0.0;

		this->state = EVAL_EXPERIMENT_STATE_VERIFY_2ND;
		this->state_iter = 0;
	}
}
