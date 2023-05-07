#include "loop_fold.h"

#include <iostream>

using namespace std;

void LoopFold::remove_inner_scope_end() {
	if (this->sum_error/(this->sequence_length+1) / this->sub_iter < 0.05) {
		cout << "REMOVE_INNER_SCOPE success" << endl;
		cout << "score: " << this->sum_error/(this->sequence_length+1) / this->sub_iter << endl;

		// don't increment clean_inner_scope_index as entry removed from inner_state_networks

		for (int i_index = 0; i_index < (int)this->curr_starting_state_networks.size(); i_index++) {
			if (this->curr_starting_state_networks[i_index] != NULL) {
				delete this->curr_starting_state_networks[i_index];
			}
		}
		this->curr_starting_state_networks = this->test_starting_state_networks;

		delete this->curr_continue_score_network;
		this->curr_continue_score_network = this->test_continue_score_network;
		this->test_continue_score_network = NULL;
		delete this->curr_continue_misguess_network;
		this->curr_continue_misguess_network = this->test_continue_misguess_network;
		this->test_continue_misguess_network = NULL;
		delete this->curr_halt_score_network;
		this->curr_halt_score_network = this->test_halt_score_network;
		this->test_halt_score_network = NULL;
		delete this->curr_halt_misguess_network;
		this->curr_halt_misguess_network = this->test_halt_misguess_network;
		this->test_halt_misguess_network = NULL;

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->curr_state_networks[f_index].size(); s_index++) {
				if (this->curr_state_networks[f_index][s_index] != NULL) {
					delete this->curr_state_networks[f_index][s_index];
				}
			}

			delete this->curr_score_networks[f_index];
		}
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
				it != this->curr_inner_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		this->curr_state_networks = this->test_state_networks;
		this->curr_score_networks = this->test_score_networks;
		this->curr_inner_state_networks = this->test_inner_state_networks;
		this->test_inner_state_networks.clear();
	} else {
		cout << "REMOVE_INNER_SCOPE fail" << endl;
		cout << "score: " << this->sum_error/(this->sequence_length+1) / this->sub_iter << endl;

		this->curr_inner_scopes_needed = this->reverse_test_inner_scopes_needed;
		this->curr_inner_contexts_needed = this->reverse_test_inner_contexts_needed;

		this->clean_inner_scope_index++;

		for (int i_index = 0; i_index < (int)this->test_starting_state_networks.size(); i_index++) {
			if (this->test_starting_state_networks[i_index] != NULL) {
				delete this->test_starting_state_networks[i_index];
			}
		}

		delete this->test_continue_score_network;
		this->test_continue_score_network = NULL;
		delete this->test_continue_misguess_network;
		this->test_continue_misguess_network = NULL;
		delete this->test_halt_score_network;
		this->test_halt_score_network = NULL;
		delete this->test_halt_misguess_network;
		this->test_halt_misguess_network = NULL;

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->test_state_networks[f_index].size(); s_index++) {
				if (this->test_state_networks[f_index][s_index] != NULL) {
					delete this->test_state_networks[f_index][s_index];
				}
			}

			delete this->test_score_networks[f_index];
		}

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_inner_state_networks.begin();
				it != this->test_inner_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		this->test_inner_state_networks.clear();
	}

	map<int, vector<vector<StateNetwork*>>>::iterator remove_scope_it = this->curr_inner_state_networks.begin();
	for (int i_index = 0; i_index < this->clean_inner_scope_index; i_index++) {
		remove_scope_it++;
	}
	while (true) {
		if (remove_scope_it == this->curr_inner_state_networks.end()) {
			this->clean_inner_scope_index = 0;
			this->clean_inner_node_index = 0;
			this->clean_inner_state_index = 0;
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
					it != this->curr_inner_state_networks.end(); it++) {
				this->curr_inner_state_networks_not_needed.insert({it->first, vector<vector<bool>>()});
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					this->curr_inner_state_networks_not_needed[it->first].push_back(
						vector<bool>(it->second[n_index].size(), false));
				}
			}
			this->test_inner_state_networks_not_needed = this->curr_inner_state_networks_not_needed;

			while (true) {
				map<int, vector<vector<bool>>>::iterator remove_network_it = this->test_inner_state_networks_not_needed.begin();
				for (int i_index = 0; i_index < this->clean_inner_scope_index; i_index++) {
					remove_network_it++;
				}
				if (remove_network_it == this->test_inner_state_networks_not_needed.end()) {
					// initialize clean
					this->clean_inner_step_index = 0;
					this->clean_inner_state_index = 0;
					this->state = LOOP_FOLD_STATE_REMOVE_INNER_NETWORK;

					cout << "ending REMOVE_INNER_SCOPE" << endl;
					remove_inner_network_transform_helper();

					break;
				}
				if (this->clean_inner_node_index >= (int)remove_network_it->second.size()) {
					this->clean_inner_node_index = 0;
					this->clean_inner_state_index = 0;
					this->clean_inner_scope_index++;
					continue;
				}
				if (this->clean_inner_state_index >= (int)remove_network_it->second[this->clean_inner_node_index].size()) {
					this->clean_inner_state_index = 0;
					this->clean_inner_node_index++;
					continue;
				}

				for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
					if (i_index >= this->sum_inner_inputs
							|| this->curr_inner_inputs_needed[i_index]) {
						this->test_starting_state_networks[i_index] = new StateNetwork(
							this->curr_starting_state_networks[i_index]);
					} else {
						this->test_starting_state_networks[i_index] = NULL;
					}
				}

				this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
				this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
				this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
				this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);

				int num_inner_networks = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_states;
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					for (int s_index = 0; s_index < num_inner_networks; s_index++) {
						if (!this->curr_state_networks_not_needed[f_index][s_index]) {
							this->test_state_networks[f_index][s_index] = new StateNetwork(this->curr_state_networks[f_index][s_index]);
						} else {
							this->test_state_networks[f_index][s_index] = NULL;
						}
					}

					this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
				}

				remove_network_it->second[this->clean_inner_node_index][this->clean_inner_state_index] = true;
				for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
						it != this->curr_inner_state_networks.end(); it++) {
					this->test_inner_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
					for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
						this->test_inner_state_networks[it->first].push_back(vector<StateNetwork*>());
						for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
							if (this->test_inner_state_networks_not_needed[it->first][n_index][s_index]) {
								this->test_inner_state_networks[it->first][n_index].push_back(NULL);
							} else {
								this->test_inner_state_networks[it->first][n_index].push_back(
									new StateNetwork(it->second[n_index][s_index]));
							}
						}
					}
				}

				cout << "ending REMOVE_INNER_SCOPE" << endl;
				cout << "starting REMOVE_INNER_SCOPE_NETWORK " << this->clean_inner_scope_index << " " << this->clean_inner_node_index << " " << this->clean_inner_state_index << endl;

				this->state = LOOP_FOLD_STATE_REMOVE_INNER_SCOPE_NETWORK;
				this->state_iter = -1;
				this->sub_iter = 0;
				this->sum_error = 0.0;

				break;
			}

			break;
		} else {
			set<int>::iterator needed_it = this->curr_inner_scopes_needed.find(remove_scope_it->first);
			if (needed_it != this->curr_inner_scopes_needed.end()) {
				this->clean_inner_scope_index++;
				remove_scope_it++;
				// continue
			} else {
				for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
					if (i_index >= this->sum_inner_inputs
							|| this->curr_inner_inputs_needed[i_index]) {
						this->test_starting_state_networks[i_index] = new StateNetwork(
							this->curr_starting_state_networks[i_index]);
					} else {
						this->test_starting_state_networks[i_index] = NULL;
					}
				}

				this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
				this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
				this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
				this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);

				int num_inner_networks = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_states;
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					for (int s_index = 0; s_index < num_inner_networks; s_index++) {
						if (!this->curr_state_networks_not_needed[f_index][s_index]) {
							this->test_state_networks[f_index][s_index] = new StateNetwork(this->curr_state_networks[f_index][s_index]);
						} else {
							this->test_state_networks[f_index][s_index] = NULL;
						}
					}

					this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
				}

				this->reverse_test_inner_scopes_needed = this->curr_inner_scopes_needed;
				this->reverse_test_inner_contexts_needed = this->curr_inner_contexts_needed;
				this->reverse_test_inner_scopes_needed.insert(remove_scope_it->first);

				for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
						it != this->curr_inner_state_networks.end(); it++) {
					if (it->first != remove_scope_it->first) {
						this->test_inner_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
						for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
							this->test_inner_state_networks[it->first].push_back(vector<StateNetwork*>());
							for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
								this->test_inner_state_networks[it->first][n_index].push_back(
									new StateNetwork(it->second[n_index][s_index]));
							}
						}
					}
				}

				cout << "ending REMOVE_INNER_SCOPE" << endl;
				cout << "starting REMOVE_INNER_SCOPE " << this->clean_inner_scope_index << endl;

				this->state = LOOP_FOLD_STATE_REMOVE_INNER_SCOPE;
				this->state_iter = -1;
				this->sub_iter = 0;
				this->sum_error = 0.0;

				break;
			}
		}
	}
}

void LoopFold::remove_inner_scope_from_load() {
	map<int, vector<vector<StateNetwork*>>>::iterator remove_scope_it = this->curr_inner_state_networks.begin();
	for (int i_index = 0; i_index < this->clean_inner_scope_index; i_index++) {
		remove_scope_it++;
	}

	this->test_starting_state_networks = vector<StateNetwork*>(this->sequence_length);
	for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
		if (i_index >= this->sum_inner_inputs
				|| this->curr_inner_inputs_needed[i_index]) {
			this->test_starting_state_networks[i_index] = new StateNetwork(
				this->curr_starting_state_networks[i_index]);
		} else {
			this->test_starting_state_networks[i_index] = NULL;
		}
	}

	this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
	this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
	this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
	this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);

	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_states;

	this->test_state_networks = vector<vector<StateNetwork*>>(this->sequence_length, vector<StateNetwork*>(num_inner_networks));
	this->test_score_networks = vector<StateNetwork*>(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < num_inner_networks; s_index++) {
			if (!this->curr_state_networks_not_needed[f_index][s_index]) {
				this->test_state_networks[f_index][s_index] = new StateNetwork(
					this->curr_state_networks[f_index][s_index]);
			} else {
				this->test_state_networks[f_index][s_index] = NULL;
			}
		}

		this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
	}

	this->reverse_test_inner_scopes_needed = this->curr_inner_scopes_needed;
	this->reverse_test_inner_contexts_needed = this->curr_inner_contexts_needed;
	this->reverse_test_inner_scopes_needed.insert(remove_scope_it->first);

	for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
			it != this->curr_inner_state_networks.end(); it++) {
		if (it->first != remove_scope_it->first) {
			this->test_inner_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				this->test_inner_state_networks[it->first].push_back(vector<StateNetwork*>());
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					this->test_inner_state_networks[it->first][n_index].push_back(
						new StateNetwork(it->second[n_index][s_index]));
				}
			}
		}
	}

	cout << "starting REMOVE_INNER_SCOPE " << this->clean_inner_scope_index << endl;

	this->state = LOOP_FOLD_STATE_REMOVE_INNER_SCOPE;
	this->state_iter = -1;
	this->sub_iter = 0;
	this->sum_error = 0.0;
}
