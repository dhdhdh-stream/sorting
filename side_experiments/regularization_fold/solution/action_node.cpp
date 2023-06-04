#include "action_node.h"

using namespace std;



void ActionNode::activate(vector<double>& flat_vals,
						  vector<State>& state_vals,
						  double& predicted_score,
						  double& scale_factor,
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	history->obs_snapshot = flat_vals.begin();
	history->state_vals_snapshot = state_vals;

	for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
		StateDefinition* curr_definition = state_vals.state_definition;
		while (curr_definition != NULL) {
			map<StateDefinition*, ScopeNetwork*>::iterator it = this->state_networks[s_index].find(curr_definition);
			if (it != this->state_networks[s_index].end()) {
				ScopeNetwork* scope_network = it->second;
				Network* network = scope_network->network;

				network->obs_input->acti_vals[0] = history->obs_snapshot;

				for (int i_index = 0; i_index < (int)scope_network->input_indexes; i_index++) {
					network->state_input->acti_vals[i_index] = history->state_vals_snapshot[
						scope_network->input_indexes[i_index]];
				}

				if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
					NetworkHistory* network_history = new NetworkHistory(network);
					network->activate(network_history);
					history->state_network_histories.push_back(ScopeNetworkHistory(scope_network, network_history));
				} else {
					network->activate();
				}

				state_vals[s_index].val += network->output->acti_vals[0];
			}

			curr_definition = curr_definition.parent;
		}

		this->score_network->state_input->acti_vals[s_index] = state_vals[s_index].val;
	}

	NetworkHistory* score_network_history = new NetworkHistory(this->score_network);
	this->score_network->activate(score_network_history);
	history->score_network_history = score_network_history;
	history->score_network_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	flat_vals.erase(flat_vals.begin());
}
