#include "branch_stub_node.h"

using namespace std;

void BranchStubNode::create_sequence_activate(
		vector<ContextLayer>& context,
		int target_num_nodes,
		int& curr_num_nodes,
		vector<map<int, int>>& state_mappings,
		int& new_num_states,
		vector<AbstractNode*>& new_nodes) {
	double obs_val;
	if (this->was_branch) {
		obs_val = -1.0;
	} else {
		obs_val = 1.0;
	}

	BranchStubNode* new_node = new BranchStubNode();
	new_nodes.push_back(new_node);
	new_node->was_branch = this->was_branch;

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

	// don't increment curr_num_nodes for BranchStubNode
}
