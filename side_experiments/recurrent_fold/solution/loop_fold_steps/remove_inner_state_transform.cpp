#include "loop_fold.h"

#include <iostream>

using namespace std;

void LoopFold::remove_inner_state_end() {
	if (this->sum_error/this->sequence_length / this->sub_state_iter < 0.01) {
		cout << "REMOVE_INNER_STATE success" << endl;
		cout << "score: " << this->sum_error/this->sequence_length / this->sub_state_iter << endl;

		for (int i_index = 0; i_index < (int)this->curr_starting_state_networks.size(); i_index++) {
			delete this->curr_starting_state_networks[i_index];
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

		this->curr_state_not_needed_locally = this->test_state_not_needed_locally;
	} else {
		cout << "REMOVE_INNER_STATE fail" << endl;
		cout << "score: " << this->sum_error/this->sequence_length / this->sub_state_iter << endl;

		for (int i_index = 0; i_index < (int)this->test_starting_state_networks.size(); i_index++) {
			delete this->test_starting_state_networks[i_index];
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

		this->test_state_not_needed_locally = this->curr_state_not_needed_locally;
	}

	this->clean_inner_state_index++;

	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_local_states
		+ this->num_input_states;
	int total_num_states = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_local_states
		+ this->num_input_states
		+ this->curr_num_new_outer_states;

	while (true) {
		if (this->clean_inner_state_index >= total_num_states) {
			this->test_num_states_cleared[this->clean_inner_step_index] = 1;

			for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
				this->test_starting_state_networks[i_index] = new StateNetwork(
					this->curr_starting_state_networks[i_index]);
			}

			this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
			this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
			this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
			this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);

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

			cout << "ending REMOVE_INNER_STATE" << endl;
			cout << "starting CLEAR_INNER_STATE " << this->clean_inner_step_index << " " << this->test_num_states_cleared[this->clean_inner_step_index] << endl;

			this->state = LOOP_FOLD_STATE_CLEAR_INNER_STATE;
			this->state_iter = 0;
			this->sub_state_iter = 0;
			this->sum_error = 0.0;

			break;
		}

		if (!this->curr_state_networks_not_needed[this->clean_inner_step_index][this->clean_inner_state_index]) {
			this->clean_inner_state_index++;
			continue;
		}

		if (this->is_inner_scope[this->clean_inner_step_index]) {
			if (this->clean_inner_state_index >= this->inner_input_start_indexes[this->clean_inner_step_index]
					&& this->clean_inner_state_index < this->inner_input_start_indexes[this->clean_inner_step_index]+this->num_inner_inputs[this->clean_inner_step_index]) {
				this->clean_inner_state_index++;
				continue;
			}
		}

		this->test_state_not_needed_locally[this->clean_inner_step_index][this->clean_inner_state_index] = true;

		for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
			this->test_starting_state_networks[i_index] = new StateNetwork(
				this->curr_starting_state_networks[i_index]);
		}

		this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
		this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
		this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
		this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);

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
		for (int s_index = 0; s_index < num_inner_networks; s_index++) {
			if (!this->curr_state_networks_not_needed[this->clean_inner_step_index][s_index]) {
				this->test_state_networks[this->clean_inner_step_index][s_index]->zero_state(this->clean_inner_state_index);
			}
		}
		this->test_score_networks[this->clean_inner_step_index]->zero_state(this->clean_inner_state_index);

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
		// don't zero inner_state_networks

		cout << "ending REMOVE_INNER_STATE" << endl;
		cout << "starting REMOVE_INNER_STATE " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

		this->state = LOOP_FOLD_STATE_REMOVE_INNER_STATE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
		this->sum_error = 0.0;

		break;
	}
}

void LoopFold::remove_inner_state_from_load() {
	this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
	this->test_state_not_needed_locally = this->curr_state_not_needed_locally;

	this->test_state_not_needed_locally[this->clean_inner_step_index][this->clean_inner_state_index] = true;

	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_local_states
		+ this->num_input_states;
	int total_num_states = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_local_states
		+ this->num_input_states
		+ this->curr_num_new_outer_states;

	this->test_starting_state_networks = vector<StateNetwork*>(this->sequence_length);
	for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
		this->test_starting_state_networks[i_index] = new StateNetwork(
			this->curr_starting_state_networks[i_index]);
	}

	this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
	this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
	this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
	this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);

	this->test_state_networks = vector<vector<StateNetwork*>>(this->sequence_length, vector<StateNetwork*>(num_inner_networks));
	this->test_score_networks = vector<StateNetwork*>(this->sequence_length);
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
	for (int s_index = 0; s_index < total_num_states; s_index++) {
		if (!this->curr_state_networks_not_needed[this->clean_inner_step_index][s_index]) {
			this->test_state_networks[this->clean_inner_step_index][s_index]->zero_state(this->clean_inner_state_index);
		}
	}
	this->test_score_networks[this->clean_inner_step_index]->zero_state(this->clean_inner_state_index);

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
	// don't zero inner_state_networks

	cout << "starting REMOVE_INNER_STATE " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

	this->state = LOOP_FOLD_STATE_REMOVE_INNER_STATE;
	this->state_iter = 0;
	this->sub_state_iter = 0;
	this->sum_error = 0.0;
}
