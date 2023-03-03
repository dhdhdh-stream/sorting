#include "fold.h"

using namespace std;

void Fold::remove_outer_scope_end() {
	// TODO: check that score networks work the same
	if (/* SUCCESS */) {
		map<int, vector<vector<StateNetwork*>>>::iterator previous_it = this->curr_outer_state_networks.begin();
		for (int i_index = 0; i_index < this->clean_outer_scope_index; i_index++) {
			previous_it++;
		}
		int clean_outer_scope_scope_id = previous_it->first;
		this->outer_scopes_checked.insert({clean_outer_scope_scope_id, false});
		this->outer_scopes_needed.clear();

		// don't increment clean_outer_scope_index as entry removed from outer_state_networks

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

		delete this->curr_starting_score_network;
		this->curr_starting_score_network = this->test_starting_score_network;
		this->test_starting_score_network = NULL;

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->curr_state_networks[f_index].size(); s_index++) {
				delete this->curr_state_networks[f_index][s_index];
			}

			delete this->curr_score_networks[f_index];
		}
		this->curr_state_networks = this->test_state_networks;
		this->curr_score_networks = this->test_score_networks;
	} else {
		map<int, vector<vector<StateNetwork*>>>::iterator previous_it = this->curr_outer_state_networks.begin();
		for (int i_index = 0; i_index < this->clean_outer_scope_index; i_index++) {
			previous_it++;
		}
		int clean_outer_scope_scope_id = previous_it->first;
		this->outer_scopes_checked.insert({clean_outer_scope_scope_id, true});
		for (set<int>::iterator needed_it = this->outer_scopes_needed.begin();
				needed_it != this->outer_scopes_needed.end(); needed_it++) {
			this->outer_scopes_checked.insert({*needed_it, true});
		}
		this->outer_scopes_needed.clear();

		this->clean_outer_scope_index++;

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.begin();
				it != this->test_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				if (it->second[n_index].size() != 0) {
					for (int s_index = 0; s_index < (int)it->second[n_index].size()) {
						delete it->second[n_index][s_index];
					}
				}
			}
		}
		this->test_outer_state_networks.clear();

		delete this->test_starting_score_network;
		this->test_starting_score_network = NULL;

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->test_state_networks[f_index].size(); s_index++) {
				delete this->test_state_networks[f_index][s_index];
			}

			delete this->test_score_networks[f_index];
		}
	}

	map<int, vector<vector<StateNetwork*>>>::iterator scope_it = this->curr_outer_state_networks.begin();
	for (int i_index = 0; i_index < this->clean_outer_scope_index; i_index++) {
		scope_it++;
	}
	while (true) {
		if (scope_it == this->curr_outer_state_networks.end()) {
			this->clean_outer_scope_index = 0;
			this->clean_outer_node_index = 0;
			this->clean_outer_state_index = 0;
			for (map<int, vector<vector<StateNetwork*>>>::iterator outer_it = this->curr_outer_state_networks.begin();
					outer_it != this->curr_outer_state_networks.end(); outer_it++) {
				this->curr_outer_state_networks_not_needed.insert({outer_it->first, vector<vector<bool>>()});
				for (int n_index = 0; n_index < (int)outer_it->second.size(); n_index++) {
					this->curr_outer_state_networks_not_needed[outer_it->first].push_back(
						vector<bool>(outer_it->second[n_index].size(), false));
				}
			}
			this->test_outer_state_networks_not_needed = this->curr_outer_state_networks_not_needed;

			while (true) {
				map<int, vector<vector<StateNetwork*>>>::iterator clean_network_it = this->test_outer_state_networks_not_needed.begin();
				for (int i_index = 0; i_index < this->clean_outer_scope_index; i_index++) {
					clean_network_it++;
				}
				if (clean_network_it == this->test_outer_state_networks_not_needed.end()) {
					// initialize clean
					int curr_total_num_states = this->sum_inner_inputs
						+ this->curr_num_new_inner_states
						+ this->num_local_states
						+ this->num_input_states
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

			break;
		} else {
			map<int, bool>::iterator checked_it = this->outer_scopes_checked.find(scope_it->first);
			if (checked_it != this->outer_scopes_checked.end()) {
				this->clean_outer_scope_index++;
				scope_it++;
				// continue
			} else {
				for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
						it != this->curr_outer_state_networks.end(); it++) {
					if (it->first != scope_it->first) {
						this->test_outer_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
						for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
							this->test_outer_state_networks[it->first].push_back(vector<StateNetwork*>());
							for (int s_index = 0; s_index < (int)it->second.size(); s_index++) {
								this->test_outer_state_networks[it->first][n_index].push_back(
									new StateNetwork(it->second[n_index][s_index]));
							}
						}
					}
				}

				this->test_starting_score_network = new StateNetwork(this->curr_starting_score_network);

				int curr_total_num_states = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_local_states
					+ this->num_input_states
					+ this->curr_num_new_outer_states;
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					for (int s_index = 0; s_index < curr_total_num_states; s_index++) {
						this->test_state_networks[f_index][s_index] = new StateNetwork(
							this->curr_state_networks[f_index][s_index]);
					}

					this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
				}

				this->state = STATE_REMOVE_OUTER_SCOPE;
				this->state_iter = 0;
				this->sum_error = 0.0;

				break;
			}
		}
	}
}
