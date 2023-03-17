#include "fold.h"

#include <iostream>

using namespace std;

void Fold::remove_inner_network_end() {
	cout << "this->test_replace_average_misguess: " << this->test_replace_average_misguess << endl;

	if (this->sum_error/this->sequence_length / this->sub_state_iter < 0.01) {
		cout << "REMOVE_INNER_NETWORK success" << endl;
		cout << "score: " << this->sum_error/this->sequence_length / this->sub_state_iter << endl;

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->curr_state_networks[f_index].size(); s_index++) {
				if (this->curr_state_networks[f_index][s_index] != NULL) {
					delete this->curr_state_networks[f_index][s_index];
				}
			}

			delete this->curr_score_networks[f_index];
		}
		this->curr_state_networks = this->test_state_networks;
		this->curr_score_networks = this->test_score_networks;

		this->curr_state_networks_not_needed = this->test_state_networks_not_needed;
	} else {
		cout << "REMOVE_INNER_NETWORK fail" << endl;
		cout << "score: " << this->sum_error/this->sequence_length / this->sub_state_iter << endl;

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->test_state_networks[f_index].size(); s_index++) {
				if (this->test_state_networks[f_index][s_index] != NULL) {
					delete this->test_state_networks[f_index][s_index];
				}
			}

			delete this->test_score_networks[f_index];
		}

		this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
	}

	this->clean_inner_state_index++;

	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_local_states
		+ this->num_sequence_input_states;
	int total_num_states = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_local_states
		+ this->num_sequence_input_states
		+ this->curr_num_new_outer_states;

	if (this->clean_inner_state_index >= total_num_states) {
		this->clean_inner_state_index = 0;

		while (true) {
			if (this->clean_inner_state_index >= total_num_states) {
				if (!this->curr_state_networks_not_needed[this->clean_inner_step_index][0]) {
					this->clean_inner_step_index++;
					if (this->clean_inner_step_index >= this->sequence_length) {
						cout << "ending REMOVE_INNER_NETWORK" << endl;
						cout << "DONE" << endl;

						this->state = FOLD_STATE_DONE;
					} else {
						this->clean_inner_state_index = 0;

						this->test_state_networks_not_needed[this->clean_inner_step_index][0] = true;

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

						cout << "ending REMOVE_INNER_NETWORK" << endl;
						cout << "starting REMOVE_INNER_NETWORK " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

						this->state = FOLD_STATE_REMOVE_INNER_NETWORK;
						this->state_iter = 0;
						this->sub_state_iter = 0;
						this->sum_error = 0.0;
					}
				} else {
					this->test_num_states_cleared[this->clean_inner_step_index] = 1;

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

					cout << "ending REMOVE_INNER_NETWORK" << endl;
					cout << "starting CLEAR_INNER_STATE " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

					this->state = FOLD_STATE_CLEAR_INNER_STATE;
					this->state_iter = 0;
					this->sub_state_iter = 0;
					this->sum_error = 0.0;
				}

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

			cout << "ending REMOVE_INNER_NETWORK" << endl;
			cout << "starting REMOVE_INNER_STATE " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

			this->state = FOLD_STATE_REMOVE_INNER_STATE;
			this->state_iter = 0;
			this->sub_state_iter = 0;
			this->sum_error = 0.0;

			break;
		}
	} else {
		this->test_state_networks_not_needed[this->clean_inner_step_index][this->clean_inner_state_index] = true;

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

		cout << "ending REMOVE_INNER_NETWORK" << endl;
		cout << "starting REMOVE_INNER_NETWORK " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

		this->state = FOLD_STATE_REMOVE_INNER_NETWORK;
		this->state_iter = 0;
		this->sub_state_iter = 0;
		this->sum_error = 0.0;
	}
}

void Fold::remove_inner_network_from_load() {
	this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
	this->test_state_not_needed_locally = this->curr_state_not_needed_locally;

	this->test_state_networks_not_needed[this->clean_inner_step_index][this->clean_inner_state_index] = true;

	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_local_states
		+ this->num_sequence_input_states;

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

	cout << "ending REMOVE_INNER_NETWORK" << endl;
	cout << "starting REMOVE_INNER_NETWORK " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

	this->state = FOLD_STATE_REMOVE_INNER_NETWORK;
	this->state_iter = 0;
	this->sub_state_iter = 0;
	this->sum_error = 0.0;
}
