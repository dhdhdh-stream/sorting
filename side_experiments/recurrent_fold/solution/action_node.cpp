#include "action_node.h"

using namespace std;

ActionNode::ActionNode(Scope* parent,
					   vector<bool> state_network_target_is_local,
					   vector<int> state_network_target_indexes,
					   vector<Network*> state_networks,
					   bool has_score_network,
					   Network* score_network) {
	this->parent = parent;
	this->type = NODE_TYPE_ACTION;

	this->state_network_target_is_local = state_network_target_is_local;
	this->state_network_target_indexes = state_network_target_indexes;
	this->state_networks = state_networks;
	this->has_score_network = has_score_network;
	this->score_network = score_network;
}

ActionNode::~ActionNode() {
	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		delete this->state_networks[s_index];
	}

	if (this->has_score_network) {
		delete this->score_network;
	}
}

void ActionNode::activate(vector<double>& local_state_vals,
						  vector<double>& input_vals,
						  vector<vector<double>>& flat_vals,
						  double& predicted_score,
						  double& scale_factor,
						  ActionNodeHistory* history) {
	vector<double> obs = *flat_vals.begin();

	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		this->state_networks[s_index]->scope_activate(obs,
													  input_vals,
													  local_state_vals);
		if (this->state_network_target_is_local[s_index]) {
			local_state_vals[this->state_network_target_indexes[s_index]] += this->state_networks[s_index]->output->acti_vals[0];
		} else {
			input_vals[this->state_network_target_indexes[s_index]] += this->state_networks[s_index]->output->acti_vals[0];
		}
	}

	flat_vals.erase(flat_vals.begin());

	if (this->has_score_network) {
		this->score_network->scope_activate(input_vals,
											local_state_vals);
		predicted_score += this->score_network->output->acti_vals[0];
	}
}
