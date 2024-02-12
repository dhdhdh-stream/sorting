#include "retrain_branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void RetrainBranchExperiment::verify_existing_activate(
		bool& is_branch,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	vector<double> input_vals(this->branch_node->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->branch_node->input_scope_contexts.size(); i_index++) {
		if (this->branch_node->input_scope_contexts[i_index].size() > 0) {
			if (this->branch_node->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->branch_node->input_node_contexts[i_index].back();
				action_node->hook_indexes.push_back(i_index);
				action_node->hook_scope_contexts.push_back(this->branch_node->input_scope_contexts[i_index]);
				action_node->hook_node_contexts.push_back(this->branch_node->input_node_contexts[i_index]);
			} else {
				BranchNode* branch_node = (BranchNode*)this->branch_node->input_node_contexts[i_index].back();
				branch_node->hook_indexes.push_back(i_index);
				branch_node->hook_scope_contexts.push_back(this->branch_node->input_scope_contexts[i_index]);
				branch_node->hook_node_contexts.push_back(this->branch_node->input_node_contexts[i_index]);
			}
		}
	}
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	input_vals_helper(scope_context,
					  node_context,
					  input_vals,
					  context[context.size() - this->branch_node->scope_context.size()].scope_history);
	for (int i_index = 0; i_index < (int)this->branch_node->input_scope_contexts.size(); i_index++) {
		if (this->branch_node->input_scope_contexts[i_index].size() > 0) {
			if (this->branch_node->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->branch_node->input_node_contexts[i_index].back();
				action_node->hook_indexes.clear();
				action_node->hook_scope_contexts.clear();
				action_node->hook_node_contexts.clear();
			} else {
				BranchNode* branch_node = (BranchNode*)this->branch_node->input_node_contexts[i_index].back();
				branch_node->hook_indexes.clear();
				branch_node->hook_scope_contexts.clear();
				branch_node->hook_node_contexts.clear();
			}
		}
	}

	double original_score = this->branch_node->original_average_score;
	for (int i_index = 0; i_index < (int)this->branch_node->linear_original_input_indexes.size(); i_index++) {
		original_score += input_vals[this->branch_node->linear_original_input_indexes[i_index]] * this->branch_node->linear_original_weights[i_index];
	}
	if (this->branch_node->original_network != NULL) {
		vector<vector<double>> original_network_input_vals(this->branch_node->original_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->branch_node->original_network_input_indexes.size(); i_index++) {
			for (int v_index = 0; v_index < (int)this->branch_node->original_network_input_indexes[i_index].size(); v_index++) {
				original_network_input_vals[i_index][v_index] = input_vals[this->branch_node->original_network_input_indexes[i_index][v_index]];
			}
		}
		this->branch_node->original_network->activate(original_network_input_vals);
		original_score += this->branch_node->original_network->output->acti_vals[0];
	}

	double branch_score = this->branch_node->branch_average_score;
	for (int i_index = 0; i_index < (int)this->branch_node->linear_branch_input_indexes.size(); i_index++) {
		branch_score += input_vals[this->branch_node->linear_branch_input_indexes[i_index]] * this->branch_node->linear_branch_weights[i_index];
	}
	if (this->branch_node->branch_network != NULL) {
		vector<vector<double>> branch_network_input_vals(this->branch_node->branch_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->branch_node->branch_network_input_indexes.size(); i_index++) {
			for (int v_index = 0; v_index < (int)this->branch_node->branch_network_input_indexes[i_index].size(); v_index++) {
				branch_network_input_vals[i_index][v_index] = input_vals[this->branch_node->branch_network_input_indexes[i_index][v_index]];
			}
		}
		this->branch_node->branch_network->activate(branch_network_input_vals);
		branch_score += this->branch_node->branch_network->output->acti_vals[0];
	}

	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	if (branch_score > original_score) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	#endif /* MDEBUG */
}

void RetrainBranchExperiment::verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (!run_helper.exceeded_limit) {
		if (run_helper.max_depth > solution->max_depth) {
			solution->max_depth = run_helper.max_depth;
		}
	}

	if (this->state == RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
			&& (int)this->o_target_val_histories.size() >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_variance = sum_score_variance / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		this->o_target_val_histories.clear();

		this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_1ST;
		this->state_iter = 0;
	} else if ((int)this->o_target_val_histories.size() >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		// this->state == RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_variance = sum_score_variance / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		this->o_target_val_histories.clear();

		this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_2ND;
		this->state_iter = 0;
	}
}
