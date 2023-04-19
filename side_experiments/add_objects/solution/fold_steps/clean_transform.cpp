#include "fold.h"

#include <iostream>

using namespace std;

void Fold::clean_transform_helper() {
	bool is_done = false;
	while (!is_done) {
		if (this->state == FOLD_STATE_REMOVE_INNER_NETWORK) {
			is_done = remove_inner_network_transform_helper();
		} else if (this->state == FOLD_STATE_REMOVE_INNER_STATE) {
			is_done = remove_inner_state_transform_helper();
		} else {
			// this->state == FOLD_STATE_CLEAR_INNER_STATE
			is_done = clear_inner_state_transform_helper();
		}
	}
}

void Fold::remove_inner_network_transform_helper() {
	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_states;
	if (this->clean_inner_state_index >= num_inner_networks) {
		this->clean_inner_state_index = 0;

		this->state = FOLD_STATE_REMOVE_INNER_STATE;

		return false;
	}

	if (this->clean_inner_state_index < this->sum_inner_inputs
			&& !this->curr_inner_inputs_needed[clean_inner_state_index]) {
		this->clean_inner_state_index++;

		return false;
	}

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

	return true;
}

void Fold::remove_inner_state_transform_helper() {
	int total_num_states = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_states
		+ this->curr_num_new_outer_states;
	if (this->clean_inner_state_index >= total_num_states) {
		this->clean_inner_state_index = 0;

		this->state = FOLD_STATE_CLEAR_INNER_STATE;

		return false;
	}

	if (!this->curr_state_networks_not_needed[this->clean_inner_step_index][this->clean_inner_state_index]) {
		this->clean_inner_state_index++;

		return false;
	}

	if (this->clean_inner_state_index < this->sum_inner_inputs
			&& !this->curr_inner_inputs_needed[clean_inner_state_index]) {
		this->clean_inner_state_index++;

		return false;
	}

	if (this->is_inner_scope[this->clean_inner_step_index]) {
		if (this->clean_inner_state_index >= this->inner_input_start_indexes[this->clean_inner_step_index]
				&& this->clean_inner_state_index < this->inner_input_start_indexes[this->clean_inner_step_index]+this->num_inner_inputs[this->clean_inner_step_index]) {
			// skip as already checked in REMOVE_INNER_INPUT

			this->clean_inner_state_index++;

			return false;
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
	// new_state_vals_initialized will be set for clean_inner_step_index + clean_inner_state_index
	// so networks will work differently for different steps

	cout << "starting REMOVE_INNER_STATE " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

	this->state_iter = 0;
	this->sub_iter = 0;
	this->sum_error = 0.0;

	return true;
}

void Fold::clear_inner_state_transform_helper() {
	// check if can clear even if just modified, as modification could just be used for score or later state

	if (this->clean_inner_state_index >= this->sum_inner_inputs+this->curr_num_new_inner_states) {
		this->clean_inner_step_index++;
		if (this->clean_inner_step_index >= this->sequence_length) {
			cout << "DONE" << endl;

			this->state = FOLD_STATE_DONE;

			return true;
		} else {
			this->clean_inner_state_index = 0;

			this->state = FOLD_STATE_REMOVE_INNER_NETWORK;

			return false;
		}
	}

	if (this->clean_inner_state_index < this->sum_inner_inputs
			&& !this->curr_inner_inputs_needed[clean_inner_state_index]) {
		this->clean_inner_state_index++;

		return false;
	}

	this->test_num_states_cleared[this->clean_inner_step_index] = this->clean_inner_state_index+1;

	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_states;
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

	cout << "starting CLEAR_INNER_STATE " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

	this->state_iter = 0;
	this->sub_iter = 0;
	this->sum_error = 0.0;

	return true;
}
