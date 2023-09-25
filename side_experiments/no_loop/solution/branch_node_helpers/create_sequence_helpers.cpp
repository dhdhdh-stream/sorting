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
			double branch_score = 0.0;
			double original_score = 0.0;
			for (int i_index = 0; i_index < (int)this->score_ids.size(); i_index++) {
				map<int, StateStatus>::iterator it = context.back().state_vals.find(this->score_ids[i_index]);
				if (it != context.back().state_vals.end()) {
					StateNetwork* last_state_network = it->second.last_state_network;
					double mean = last_state_network->ending_mean;
					double standard_deviation = last_state_network->ending_standard_deviation;
					double normalized = ((it->second.val - mean) / standard_deviation) * last_state_network->correlation_to_end;
					branch_score += normalized*this->branch_score_weights[i_index];
					original_score += normalized*this->original_score_weights[i_index];
				}
			}

			if (branch_score > original_score) {
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

			vector<double> state_snapshots(this->local_state_ids.size());
			for (int n_index = 0; n_index < (int)this->local_state_ids.size(); n_index++) {
				map<int, StateStatus>::iterator it = context.back().state_vals.find(this->local_state_ids[n_index]);
				if (it == context.back().state_vals.end()) {
					it = context.back().state_vals.insert({this->local_state_ids[n_index], StateStatus()}).first;
				}
				StateNetwork* state_network = this->states[n_index]->networks[this->network_indexes[n_index]];
				if (this->obs_ids[n_index] == -1) {
					state_network->activate(obs_val,
											it->second);
				} else {
					state_network->activate(state_snapshots[this->obs_ids[n_index]],
											it->second);
				}
				state_snapshots[n_index] = it->second.val;

				int new_state_id;
				map<int, int>::iterator state_it = state_mappings.back().find(this->local_state_ids[n_index]);
				if (state_it != state_mappings.back().end()) {
					new_state_id = state_it->second;
				} else {
					state_mappings.back()[this->local_state_ids[n_index]] = new_num_states;
					new_state_id = new_num_states;
					new_num_states++;
				}
				new_node->local_state_ids.push_back(new_state_id);
				new_node->obs_ids.push_back(this->obs_ids[n_index]);
				new_node->states.push_back(this->states[n_index]);
				new_node->network_indexes.push_back(this->network_indexes[n_index]);
			}

			// don't worry about score_state_networks

			// don't increment curr_num_nodes for BranchNode
		}
	} else {
		is_branch = false;
	}
}
