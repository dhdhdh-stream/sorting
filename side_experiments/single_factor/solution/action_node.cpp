#include "action_node.h"

using namespace std;



void ActionNode::explore_activate(vector<double>& flat_vals,
								  vector<ContextLayer>& context,
								  vector<vector<AbstractNodeHistory*>>& node_histories) {
	ActionNodeHistory* action_node_history = new ActionNodeHistory(this);
	action_node_history->obs_snapshot = flat_vals[0];
	node_histories.back().push_back(action_node_history);

	for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
		if (context.back().state_weights[this->state_network_indexes[n_index]] != 0.0) {
			this->state_networks[n_index]->activate(flat_vals[0],
													context.back().state_vals->at(this->state_network_indexes[n_index]));
		}
	}

	flat_vals.erase(flat_vals.begin());
}

void ActionNode::update_activate(vector<double>& flat_vals,
								 vector<ContextLayer>& context) {
	for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
		if (context.back().state_weights[this->state_network_indexes[n_index]] != 0.0) {
			this->state_networks[n_index]->activate(flat_vals[0],
													context.back().state_vals->at(this->state_network_indexes[n_index]));
		}
	}

	flat_vals.erase(flat_vals.begin());
}


