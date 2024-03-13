#include "branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::root_verify_activate(
		vector<int>& context_match_indexes,
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
			action_node->hook_indexes.push_back(i_index);
			action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
			action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
			action_node->hook_is_fuzzy_match.push_back(this->input_is_fuzzy_match[i_index]);
			action_node->hook_strict_root_indexes.push_back(this->input_strict_root_indexes[i_index]);
		} else {
			BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
			branch_node->hook_indexes.push_back(i_index);
			branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
			branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
			branch_node->hook_is_fuzzy_match.push_back(this->input_is_fuzzy_match[i_index]);
			branch_node->hook_strict_root_indexes.push_back(this->input_strict_root_indexes[i_index]);
		}
	}
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	input_vals_helper(scope_context,
					  node_context,
					  true,
					  0,
					  context_match_indexes,
					  input_vals,
					  context[context_match_indexes[0]].scope_history);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
			action_node->hook_indexes.clear();
			action_node->hook_scope_contexts.clear();
			action_node->hook_node_contexts.clear();
			action_node->hook_is_fuzzy_match.clear();
			action_node->hook_strict_root_indexes.clear();
		} else {
			BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
			branch_node->hook_indexes.clear();
			branch_node->hook_scope_contexts.clear();
			branch_node->hook_node_contexts.clear();
			branch_node->hook_is_fuzzy_match.clear();
			branch_node->hook_strict_root_indexes.clear();
		}
	}

	double existing_predicted_score = this->existing_average_score
		+ this->original_bias * this->existing_score_standard_deviation;
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
	bool decision_is_branch = new_predicted_score > existing_predicted_score;
	#endif /* MDEBUG */

	BranchNodeHistory* branch_node_history = new BranchNodeHistory(this->branch_node);
	context.back().scope_history->node_histories.push_back(branch_node_history);
	if (decision_is_branch) {
		branch_node_history->is_branch = true;

		if (this->throw_id != -1) {
			run_helper.throw_id = -1;
		}

		if (this->best_step_types.size() == 0) {
			curr_node = this->exit_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else if (this->best_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
				curr_node = this->best_existing_scopes[0];
			} else {
				curr_node = this->best_potential_scopes[0];
			}
		}
	} else {
		branch_node_history->is_branch = false;
	}
}
