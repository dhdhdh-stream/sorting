#if defined(MDEBUG) && MDEBUG

#include "retrain_branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "network.h"
#include "problem.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void RetrainBranchExperiment::capture_verify_activate(
		bool& is_branch,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_scope_contexts[i_index].size() > 0) {
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
	}
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	input_vals_helper(scope_context,
					  node_context,
					  input_vals,
					  context[context.size() - this->branch_node->scope_context.size()].scope_history);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_scope_contexts[i_index].size() > 0) {
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

	this->verify_original_scores.push_back(original_predicted_score);
	this->verify_branch_scores.push_back(branch_predicted_score);

	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
}

void RetrainBranchExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		this->result = EXPERIMENT_RESULT_SUCCESS;
	}
}

#endif /* MDEBUG */