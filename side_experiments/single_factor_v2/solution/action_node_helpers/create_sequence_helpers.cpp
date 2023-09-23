#include "action_node.h"

using namespace std;

void ActionNode::create_sequence_activate(
		vector<double>& flat_vals,
		vector<ContextLayer>& context,
		int target_num_nodes,
		int& curr_num_nodes,
		vector<map<int, int>>& state_mappings,
		int& new_num_states,
		vector<AbstractNode*>& new_nodes) {
	double obs_val = flat_vals[0];
	flat_vals.erase(flat_vals.begin());

	ActionNode* new_node = new ActionNode();
	new_nodes.push_back(new_node);

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

	curr_num_nodes++;
}
