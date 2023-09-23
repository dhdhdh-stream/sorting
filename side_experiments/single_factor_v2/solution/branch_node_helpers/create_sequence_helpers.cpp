#include "branch_node.h"

using namespace std;

void BranchNode::create_sequence_activate(
		vector<ContextLayer>& context,
		int target_num_nodes,
		int& curr_num_nodes,
		vector<map<int, int>>& state_mappings,
		int& new_num_states,
		vector<AbstractNode*>& new_nodes,
		bool& is_branch) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->branch_scope_context.size()-1; c_index++) {
			if (this->branch_scope_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].scope_id
					|| this->branch_node_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].node_id) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->branch_is_pass_through) {
			is_branch = true;
		} else {
			for (int i_index = 0; i_index < (int)this->score_network_input_ids.size(); i_index++) {
				double val;
				map<int, double>::iterator it = context.back().state_vals.find(this->score_network_input_ids[i_index]);
				if (it != context.back().state_vals.end()) {
					val = it->second;
				} else {
					val = 0.0;
				}
				this->branch_score_network->input->acti_vals[i_index] = val;
				this->original_score_network->input->acti_vals[i_index] = val;
			}
			this->branch_score_network->activate();
			this->original_score_network->activate();

			if (this->branch_score_network->acti_vals[0] > this->original_score_network->acti_vals[0]) {
				is_branch = true;
			} else {
				is_branch = false;
			}

			double obs_val;
			if (is_branch) {
				obs_val = -1.0;
			} else {
				obs_val = 1.0;
			}

			BranchStubNode* new_node = new BranchStubNode();
			new_nodes.push_back(new_node);
			new_node->was_branch = is_branch;

			for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
				map<int, double>::iterator it = context.back().state_vals.find(this->state_ids[n_index]);
				if (it == context.back().state_vals.end()) {
					it = context.back().state_vals.insert({this->state_ids[n_index], 0.0}).first;
				}

				this->state_networks[n_index]->activate(obs_val,
														it->second);

				int new_state_id;
				map<int, int>::iterator state_it = state_mappings.back().find(this->state_ids[n_index]);
				if (state_it != state_mappings.back().end()) {
					new_state_id = state_it->second;
				} else {
					state_mappings.back()[this->state_ids[n_index]] = new_num_states;
					new_state_id = new_num_states;
					new_num_states++;
				}
				new_node->state_ids.push_back(new_state_id);
				new_node->state_networks.push_back(new StateNetwork(this->state_networks[n_index]));
			}

			// don't worry about score_state_networks

			// don't increment curr_num_nodes for BranchNode
		}
	} else {
		is_branch = false;
	}
}
