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
	double obs_snapshot = flat_vals[0];
	flat_vals.erase(flat_vals.begin());

	ActionNode* new_node = new ActionNode();
	new_nodes.push_back(new_node);

	vector<double> state_snapshots(this->local_state_ids.size());
	for (int n_index = 0; n_index < (int)this->local_state_ids.size(); n_index++) {
		map<int, StateStatus>::iterator it = context.back().state_vals.find(this->local_state_ids[n_index]);
		if (it == context.back().state_vals.end()) {
			it = context.back().state_vals.insert({this->local_state_ids[n_index], StateStatus()}).first;
		}
		StateNetwork* state_network = this->states[n_index]->networks[this->network_indexes[n_index]];
		if (this->obs_ids[n_index] == -1) {
			state_network->activate(obs_snapshot,
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

	curr_num_nodes++;
}
