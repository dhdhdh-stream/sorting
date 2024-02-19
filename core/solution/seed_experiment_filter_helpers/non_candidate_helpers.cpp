#include "seed_experiment_filter.h"

using namespace std;

void SeedExperimentFilter::non_candidate_activate(
		AbstractNode*& curr_node,
		vector<ContextLayer>& context) {
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
					  context[context.size() - this->scope_context.size()].scope_history);
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

	double score = this->average_confidence;
	for (int l_index = 0; l_index < (int)this->linear_weights.size(); l_index++) {
		score += input_vals[l_index] * this->linear_weights[l_index];
	}
	if (this->network != NULL) {
		vector<vector<double>> network_input_vals(this->network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)this->network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = input_vals[this->network_input_indexes[i_index][v_index]];
			}
		}
		this->network->activate(network_input_vals);
		score += this->network->output->acti_vals[0];
	}

	if (score < FILTER_CONFIDENCE_THRESHOLD) {
		if (this->step_types.size() == 0) {
			curr_node = this->exit_next_node;
		} else {
			if (this->step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->actions[0];
			} else if (this->step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
				curr_node = this->existing_scopes[0];
			} else {
				curr_node = this->potential_scopes[0];
			}
		}
	}
}
