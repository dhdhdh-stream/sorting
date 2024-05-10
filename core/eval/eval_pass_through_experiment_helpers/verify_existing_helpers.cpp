#include "eval_pass_through_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void EvalPassThroughExperiment::verify_existing_back_activate(
		ScopeHistory*& subscope_history,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_histories.back();

	vector<double> input_vals(solution->eval->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)solution->eval->input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = subscope_history->node_histories.find(
			solution->eval->input_node_contexts[i_index]);
		if (it != subscope_history->node_histories.end()) {
			switch (solution->eval->input_node_contexts[i_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					input_vals[i_index] = action_node_history->obs_snapshot[solution->eval->input_obs_indexes[i_index]];
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

void EvalPassThroughExperiment::verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_histories.back();

	for (int p_index = 0; p_index < (int)history->predicted_scores.size(); p_index++) {
		double misguess = (target_val - history->predicted_scores[p_index]) * (target_val - history->predicted_scores[p_index]);
		this->misguess_histories.push_back(misguess);
	}

	this->state_iter++;
	if (this->state == EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
			&& this->state_iter >= VERIFY_1ST_NUM_DATAPOINTS) {
		int num_instances = (int)this->misguess_histories.size();

		double sum_misguesses = 0.0;
		for (int m_index = 0; m_index < num_instances; m_index++) {
			sum_misguesses += this->misguess_histories[m_index];
		}
		this->existing_average_misguess = sum_misguesses / num_instances;

		this->misguess_histories.clear();

		this->misguess_histories.reserve(VERIFY_1ST_NUM_DATAPOINTS);

		this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST;
		this->state_iter = 0;
	} else if (this->state_iter >= VERIFY_2ND_NUM_DATAPOINTS) {
		int num_instances = (int)this->misguess_histories.size();

		double sum_misguesses = 0.0;
		for (int m_index = 0; m_index < num_instances; m_index++) {
			sum_misguesses += this->misguess_histories[m_index];
		}
		this->existing_average_misguess = sum_misguesses / num_instances;

		this->misguess_histories.clear();

		this->misguess_histories.reserve(VERIFY_2ND_NUM_DATAPOINTS);

		this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND;
		this->state_iter = 0;
	}
}
