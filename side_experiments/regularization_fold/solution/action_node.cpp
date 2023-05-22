#include "action_node.h"

using namespace std;



void ActionNode::activate(vector<double>& flat_vals,
						  vector<Object>& obj_vals,
						  double& predicted_score,
						  double& scale_factor,
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	history->obs_snapshot = flat_vals.begin();
	history->obj_vals_snapshot = obj_vals;

	for (int o_index = 0; o_index < (int)obj_vals.size(); o_index++) {
		ObjectDefinition* curr_definition = obj_vals[o_index].definition;
		while (true) {
			map<ObjectDefinition*, vector<ObjectNetwork*>>::iterator it = this->networks.find(curr_definition);
			if (it != this->networks.end()) {
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					ObjectNetwork* object_network = it->second[n_index];
					Network* network = object_network->network;

					network->obs_input->acti_vals[0] = history->obs_snapshot;

					for (int s_index = 0; s_index < network->state_size; s_index++) {
						network->state_input->acti_vals[s_index] = history->obj_vals_snapshot[object_network->object_index[s_index]]
							->state_vals[object_network->state_index[s_index]];
					}

					NetworkHistory* network_history = new NetworkHistory(network);
					network->activate(network_history);
					history->network_histories.push_back(ObjectNetworkHistory(object_network,
																			  network_history));

					obj_vals[o_index]->state_vals[object_network->target_index] += network->output->acti_vals[0];
				}
			}

			curr_definition = curr_definition->parent;
			if (curr_definition == NULL) {
				break;
			}
		}
	}

	// using updated obj_vals
	

	flat_vals.erase(flat_vals.begin());
}
