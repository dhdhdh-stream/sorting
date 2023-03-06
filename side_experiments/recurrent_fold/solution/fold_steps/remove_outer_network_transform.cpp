#include "fold.h"

using namespace std;

void Fold::remove_outer_network_end() {
	if (this->sum_error/10000 < 0.01) {
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
				it != this->curr_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		this->curr_outer_state_networks = this->test_outer_state_networks;
		this->test_outer_state_networks.clear();

		this->curr_outer_state_networks_not_needed = this->test_outer_state_networks_not_needed;
	} else {
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.begin();
				it != this->test_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		this->test_outer_state_networks.clear();
	}

	this->clean_outer_state_index++;

	while (true) {
		map<int, vector<vector<StateNetwork*>>>::iterator clean_network_it = this->test_outer_state_networks_not_needed.begin();
		for (int i_index = 0; i_index < this->clean_outer_scope_index; i_index++) {
			clean_network_it++;
		}
		if (clean_network_it == this->test_outer_state_networks_not_needed.end()) {
			// initialize clean
			int curr_total_num_states = this->sum_inner_inputs
				+ this->curr_num_new_inner_states
				+ this->num_sequence_local_states
				+ this->num_sequence_input_states
				+ this->curr_num_new_outer_states;

			this->clean_inner_step_index = 0;
			this->clean_inner_state_index = 0;

			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				this->curr_state_networks_not_needed.push_back(vector<bool>(curr_total_num_states, false));
				this->test_state_networks_not_needed.push_back(vector<bool>(curr_total_num_states, false));

				this->curr_state_not_needed_locally.push_back(vector<bool>(curr_total_num_states, false));
				this->test_state_not_needed_locally.push_back(vector<bool>(curr_total_num_states, false));

				this->curr_num_states_cleared.push_back(0);
				this->test_num_states_cleared.push_back(0);
			}

			this->test_state_networks_not_needed[0][0] = true;

			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				for (int s_index = 0; s_index < curr_total_num_states; s_index++) {
					if (!this->test_state_networks_not_needed[f_index][s_index]) {
						this->test_state_networks[f_index][s_index] = new StateNetwork(this->curr_state_networks[f_index][s_index]);
					} else {
						this->test_state_networks[f_index][s_index] = NULL;
					}
				}

				this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
			}

			this->state = STATE_REMOVE_INNER_NETWORK;
			this->state_iter = 0;
			this->sum_error = 0.0;

			break;
		}
		if (this->clean_outer_node_index >= (int)clean_network_it->second.size()) {
			this->clean_outer_node_index = 0;
			this->clean_outer_state_index = 0;
			this->clean_outer_scope_index++;
			continue;
		}
		if (this->clean_outer_state_index >= (int)clean_network_it->second[this->clean_outer_node_index].size()) {
			this->clean_outer_state_index = 0;
			this->clean_outer_node_index++;
			continue;
		}

		clean_network_it->second[this->clean_outer_node_index][this->clean_outer_state_index] = true;
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
				it != this->curr_outer_state_networks.end(); it++) {
			this->test_outer_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
			for (int s_index = 0; s_index < (int)it->second.size(); s_index++) {
				if (this->test_outer_state_networks_not_needed[it->first][n_index][s_index]) {
					this->test_outer_state_networks[it->first][n_index].push_back(NULL);
				} else {
					this->test_outer_state_networks[it->first][n_index].push_back(
						new StateNetwork(it->second[n_index][s_index]));
				}
			}
		}

		// don't special case starting_score_network

		// don't special case inner

		this->state = STATE_REMOVE_OUTER_NETWORK;
		this->state_iter = 0;
		this->sum_error = 0.0;

		break;
	}
}
