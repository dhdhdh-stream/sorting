#include "fold.h"

#include <iostream>

using namespace std;

void LoopFold::remove_inner_network_transform_helper() {
	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_states;
	while (true) {
		if (this->clean_inner_step_index >= this->sequence_length) {
			cout << "DONE" << endl;

			this->state = LOOP_FOLD_STATE_DONE;

			break;
		}

		if (this->clean_inner_state_index >= num_inner_networks) {
			this->clean_inner_state_index = 0;
			this->clean_inner_step_index++;
			continue;
		}

		if (this->clean_inner_state_index < this->sum_inner_inputs
				&& !this->curr_inner_inputs_needed[clean_inner_state_index]) {
			this->clean_inner_state_index++;
			continue;
		}

		this->test_state_networks_not_needed[this->clean_inner_step_index][this->clean_inner_state_index] = true;

		for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
			// this->test_starting_state_networks previously cleared
			if (i_index >= this->sum_inner_inputs
					|| this->curr_inner_inputs_needed[i_index]) {
				this->test_starting_state_networks[i_index] = new StateNetwork(this->curr_starting_state_networks[i_index]);
			} else {
				this->test_starting_state_networks[i_index] = NULL;
			}
		}

		this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
		this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
		this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
		this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < num_inner_networks; s_index++) {
				if (!this->test_state_networks_not_needed[f_index][s_index]) {
					this->test_state_networks[f_index][s_index] = new StateNetwork(this->curr_state_networks[f_index][s_index]);
				} else {
					this->test_state_networks[f_index][s_index] = NULL;
				}
			}

			this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
		}

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
				it != this->curr_inner_state_networks.end(); it++) {
			this->test_inner_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				this->test_inner_state_networks[it->first].push_back(vector<StateNetwork*>());
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					if (this->curr_inner_state_networks_not_needed[it->first][n_index][s_index]) {
						this->test_inner_state_networks[it->first][n_index].push_back(NULL);
					} else {
						this->test_inner_state_networks[it->first][n_index].push_back(
							new StateNetwork(it->second[n_index][s_index]));
					}
				}
			}
		}

		cout << "starting REMOVE_INNER_NETWORK " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

		this->state_iter = 0;
		this->sub_iter = 0;
		this->sum_error = 0.0;

		break;
	}
}

void LoopFold::remove_inner_network_end() {
	cout << "this->curr_average_score: " << this->curr_average_score << endl;

	if (this->sum_error/this->sequence_length / this->sub_iter < 0.05) {
		cout << "REMOVE_INNER_NETWORK success" << endl;
		cout << "score: " << this->sum_error/this->sequence_length / this->sub_iter << endl;

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
					if (it->second[n_index][s_index] != NULL) {
						delete it->second[n_index][s_index];
					}
				}
			}
		}
		this->curr_state_networks = this->test_state_networks;
		this->curr_score_networks = this->test_score_networks;
		this->curr_inner_state_networks = this->test_inner_state_networks;
		this->test_inner_state_networks.clear();

		this->curr_state_networks_not_needed = this->test_state_networks_not_needed;
	} else {
		cout << "REMOVE_INNER_NETWORK fail" << endl;
		cout << "score: " << this->sum_error/this->sequence_length / this->sub_iter << endl;

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
					if (it->second[n_index][s_index] != NULL) {
						delete it->second[n_index][s_index];
					}
				}
			}
		}
		this->test_inner_state_networks.clear();

		this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
	}

	this->clean_inner_state_index++;

	cout << "ending REMOVE_INNER_NETWORK" << endl;
	remove_inner_network_transform_helper();
}

void LoopFold::remove_inner_network_from_load() {
	this->test_state_networks_not_needed = this->curr_state_networks_not_needed;

	this->test_state_networks_not_needed[this->clean_inner_step_index][this->clean_inner_state_index] = true;

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
			if (!this->test_state_networks_not_needed[f_index][s_index]) {
				this->test_state_networks[f_index][s_index] = new StateNetwork(this->curr_state_networks[f_index][s_index]);
			} else {
				this->test_state_networks[f_index][s_index] = NULL;
			}
		}

		this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
	}

	for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
			it != this->curr_inner_state_networks.end(); it++) {
		this->test_inner_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			this->test_inner_state_networks[it->first].push_back(vector<StateNetwork*>());
			for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
				if (this->curr_inner_state_networks_not_needed[it->first][n_index][s_index]) {
					this->test_inner_state_networks[it->first][n_index].push_back(NULL);
				} else {
					this->test_inner_state_networks[it->first][n_index].push_back(
						new StateNetwork(it->second[n_index][s_index]));
				}
			}
		}
	}

	cout << "starting REMOVE_INNER_NETWORK " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

	this->state = LOOP_FOLD_STATE_REMOVE_INNER_NETWORK;
	this->state_iter = 0;
	this->sub_iter = 0;
	this->sum_error = 0.0;
}
