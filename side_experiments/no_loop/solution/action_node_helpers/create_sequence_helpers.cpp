#include "action_node.h"

using namespace std;

void ActionNode::create_sequence_activate(
		vector<double>& flat_vals,
		vector<ContextLayer>& context,
		int target_num_nodes,
		int& curr_num_nodes,
		Sequence* new_sequence,
		vector<map<pair<bool,int>, int>>& state_mappings,
		int& new_num_input_states,
		vector<AbstractNode*>& new_nodes) {
	double obs_snapshot = flat_vals[0];
	flat_vals.erase(flat_vals.begin());

	ActionNode* new_node = new ActionNode();
	new_nodes.push_back(new_node);

	for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
		if (this->state_is_local[n_index]) {
			map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->state_indexes[n_index]);
			if (it == context.back().local_state_vals.end()) {
				it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
			state_network->activate(obs_snapshot,
									it->second);

			new_node->state_is_local.push_back(false);
			map<pair<bool,int>, int>::iterator new_state_it = state_mappings.back().find({true, this->state_indexes[n_index]});
			if (new_state_it != state_mappings.back().end()) {
				new_node->state_indexes.push_back(new_state_it->second);
			} else {
				int new_state_index;
				state_mappings.back()[{true, this->state_indexes[n_index]}] = new_num_input_states;
				new_state_index = new_num_input_states;
				new_num_input_states++;

				new_node->state_indexes.push_back(new_state_index);

				new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
				new_sequence->input_inner_indexes.push_back(new_state_index);
				new_sequence->input_scope_depths.push_back(-1);
				new_sequence->input_outer_is_local.push_back(-1);
				new_sequence->input_outer_indexes.push_back(-1);
				new_sequence->input_init_vals.push_back(0.0);
			}
			new_node->state_defs.push_back(this->state_defs[n_index]);
			new_node->state_network_indexes.push_back(this->state_network_indexes[n_index]);
		} else {
			map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->state_indexes[n_index]);
			if (it != context.back().input_state_vals.end()) {
				StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				state_network->activate(obs_snapshot,
										it->second);

				map<pair<bool,int>, int>::iterator new_state_it = state_mappings.back().find({false, this->state_indexes[n_index]});
				// new_state_it != state_mappings.back().end()
				new_node->state_is_local.push_back(false);
				new_node->state_indexes.push_back(new_state_it->second);
				new_node->state_defs.push_back(this->state_defs[n_index]);
				new_node->state_network_indexes.push_back(this->state_network_indexes[n_index]);
			}
		}
	}

	// don't worry about score_state

	curr_num_nodes++;
}
