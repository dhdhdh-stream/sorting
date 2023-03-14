#include "fold.h"

#include <iostream>

using namespace std;

void Fold::remove_outer_scope_end() {
	if (this->sum_error/(this->sequence_length+1) / this->sub_state_iter < 0.01) {
		cout << "REMOVE_OUTER_SCOPE success" << endl;
		cout << "score: " << this->sum_error/(this->sequence_length+1) / this->sub_state_iter << endl;

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
		cout << "REMOVE_OUTER_SCOPE fail" << endl;
		cout << "score: " << this->sum_error/(this->sequence_length+1) / this->sub_state_iter << endl;

		this->curr_outer_scopes_needed = this->reverse_test_outer_scopes_needed;
		this->curr_outer_contexts_needed = this->reverse_test_outer_contexts_needed;

		this->clean_outer_scope_index++;

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.begin();
				it != this->test_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				if (it->second[n_index].size() != 0) {
					for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
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
				map<int, vector<vector<bool>>>::iterator clean_network_it = this->test_outer_state_networks_not_needed.begin();
				for (int i_index = 0; i_index < this->clean_outer_scope_index; i_index++) {
					clean_network_it++;
				}
				if (clean_network_it == this->test_outer_state_networks_not_needed.end()) {
					if (this->curr_outer_state_networks.size() == 0) {
						// no new_outer_state edge case
						this->curr_num_new_outer_states = 0;
						this->curr_starting_score_network->remove_new_outer();
						int curr_total_num_states = this->sum_inner_inputs
							+ this->curr_num_new_inner_states
							+ this->num_sequence_local_states
							+ this->num_sequence_input_states;
						// without this->curr_num_new_outer_states
						for (int f_index = 0; f_index < this->sequence_length; f_index++) {
							for (int s_index = 0; s_index < curr_total_num_states; s_index++) {
								this->curr_state_networks[f_index][s_index]->remove_new_outer();
							}

							for (int s_index = curr_total_num_states; s_index < (int)this->curr_state_networks[f_index].size(); s_index++) {
								delete this->curr_state_networks[f_index][s_index];
							}
							this->curr_state_networks[f_index].erase(this->curr_state_networks[f_index].begin()+curr_total_num_states, this->curr_state_networks[f_index].end());

							this->curr_score_networks[f_index]->remove_new_outer();
						}

						// set this->test_state_networks shape
						this->test_state_networks = this->curr_state_networks;
					}

					// initialize clean
					this->clean_inner_step_index = 0;
					this->clean_inner_state_index = 0;

					this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
					this->test_state_not_needed_locally = this->curr_state_not_needed_locally;
					this->test_num_states_cleared = this->curr_num_states_cleared;

					this->test_state_networks_not_needed[0][0] = true;

					int curr_total_num_states = this->sum_inner_inputs
						+ this->curr_num_new_inner_states
						+ this->num_sequence_local_states
						+ this->num_sequence_input_states
						+ this->curr_num_new_outer_states;
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

					cout << "ending REMOVE_OUTER_SCOPE" << endl;
					cout << "starting REMOVE_INNER_NETWORK " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

					this->state = FOLD_STATE_REMOVE_INNER_NETWORK;
					this->state_iter = 0;
					this->sub_state_iter = 0;
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
					for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
						this->test_outer_state_networks[it->first].push_back(vector<StateNetwork*>());
						for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
							if (this->test_outer_state_networks_not_needed[it->first][n_index][s_index]) {
								this->test_outer_state_networks[it->first][n_index].push_back(NULL);
							} else {
								this->test_outer_state_networks[it->first][n_index].push_back(
									new StateNetwork(it->second[n_index][s_index]));
							}
						}
					}
				}

				// don't special case starting_score_network

				// don't special case inner

				cout << "ending REMOVE_OUTER_SCOPE" << endl;
				cout << "starting REMOTE_OUTER_NETWORK " << this->clean_outer_scope_index << " " << this->clean_outer_node_index << " " << this->clean_outer_state_index << endl;

				this->state = FOLD_STATE_REMOVE_OUTER_NETWORK;
				this->state_iter = 0;
				this->sub_state_iter = 0;
				this->sum_error = 0.0;

				break;
			}

			break;
		} else {
			set<int>::iterator needed_it = this->curr_outer_scopes_needed.find(scope_it->first);
			if (needed_it != this->curr_outer_scopes_needed.end()) {
				this->clean_outer_scope_index++;
				scope_it++;
				// continue
			} else {
				this->reverse_test_outer_scopes_needed = this->curr_outer_scopes_needed;
				this->reverse_test_outer_contexts_needed = this->curr_outer_contexts_needed;
				this->reverse_test_outer_scopes_needed.insert(scope_it->first);

				for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
						it != this->curr_outer_state_networks.end(); it++) {
					if (it->first != scope_it->first) {
						this->test_outer_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
						for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
							this->test_outer_state_networks[it->first].push_back(vector<StateNetwork*>());
							for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
								this->test_outer_state_networks[it->first][n_index].push_back(
									new StateNetwork(it->second[n_index][s_index]));
							}
						}
					}
				}

				this->test_starting_score_network = new StateNetwork(this->curr_starting_score_network);

				int curr_total_num_states = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_sequence_local_states
					+ this->num_sequence_input_states
					+ this->curr_num_new_outer_states;
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					for (int s_index = 0; s_index < curr_total_num_states; s_index++) {
						this->test_state_networks[f_index][s_index] = new StateNetwork(
							this->curr_state_networks[f_index][s_index]);
					}

					this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
				}

				cout << "ending REMOVE_OUTER_SCOPE" << endl;
				cout << "starting REMOVE_OUTER_SCOPE " << this->clean_outer_scope_index << endl;

				this->state = FOLD_STATE_REMOVE_OUTER_SCOPE;
				this->state_iter = 0;
				this->sub_state_iter = 0;
				this->sum_error = 0.0;

				break;
			}
		}
	}
}
