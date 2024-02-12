#include "retrain_branch_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void RetrainBranchExperiment::verify_activate(bool& is_branch,
											  vector<ContextLayer>& context,
											  RunHelper& run_helper) {
	vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
			action_node->hook_indexes.push_back(i_index);
			action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
			action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
		} else {
			BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
			branch_node->hook_indexes.push_back(i_index);
			branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
			branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
		}
	}
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	input_vals_helper(scope_context,
					  node_context,
					  input_vals,
					  context[context.size() - this->branch_node->scope_context.size()].scope_history);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
			action_node->hook_indexes.clear();
			action_node->hook_scope_contexts.clear();
			action_node->hook_node_contexts.clear();
		} else {
			BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
			branch_node->hook_indexes.clear();
			branch_node->hook_scope_contexts.clear();
			branch_node->hook_node_contexts.clear();
		}
	}

	double original_predicted_score = this->original_average_score;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		original_predicted_score += input_vals[i_index] * this->original_linear_weights[i_index];
	}
	if (this->original_network != NULL) {
		vector<vector<double>> original_network_input_vals(this->original_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->original_network_input_indexes.size(); i_index++) {
			original_network_input_vals[i_index] = vector<double>(this->original_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->original_network_input_indexes[i_index].size(); s_index++) {
				original_network_input_vals[i_index][s_index] = input_vals[this->original_network_input_indexes[i_index][s_index]];
			}
		}
		this->original_network->activate(original_network_input_vals);
		original_predicted_score += this->original_network->output->acti_vals[0];
	}

	double branch_predicted_score = this->branch_average_score;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		branch_predicted_score += input_vals[i_index] * this->branch_linear_weights[i_index];
	}
	if (this->branch_network != NULL) {
		vector<vector<double>> branch_network_input_vals(this->branch_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->branch_network_input_indexes.size(); i_index++) {
			branch_network_input_vals[i_index] = vector<double>(this->branch_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->branch_network_input_indexes[i_index].size(); s_index++) {
				branch_network_input_vals[i_index][s_index] = input_vals[this->branch_network_input_indexes[i_index][s_index]];
			}
		}
		this->branch_network->activate(branch_network_input_vals);
		branch_predicted_score += this->branch_network->output->acti_vals[0];
	}

	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	is_branch = branch_predicted_score > original_predicted_score;
	#endif /* MDEBUG */
}

void RetrainBranchExperiment::verify_backprop(double target_val) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state == RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_1ST
			&& this->state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		this->combined_score /= (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double combined_improvement = this->combined_score - this->existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));

		if (combined_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			this->combined_score = 0.0;

			this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

			this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	} else if (this->state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		// this->state == RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_2ND
		this->combined_score /= (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		double score_standard_deviation = sqrt(this->existing_score_variance);
		double combined_improvement = this->combined_score - this->existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (combined_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			cout << "Retrain Branch" << endl;
			cout << "verify" << endl;

			cout << "this->combined_score: " << this->combined_score << endl;
			cout << "this->existing_average_score: " << this->existing_average_score << endl;
			cout << "score_standard_deviation: " << score_standard_deviation << endl;
			cout << "combined_improvement_t_score: " << combined_improvement_t_score << endl;

			cout << endl;

			#if defined(MDEBUG) && MDEBUG
			this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
			this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

			this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
			this->state_iter = 0;
			#else
			this->result = EXPERIMENT_RESULT_SUCCESS;
			#endif /* MDEBUG */
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
