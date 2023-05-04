#include "fold.h"

#include <iostream>

using namespace std;

void Fold::clear_inner_state_end() {
	// if (this->sum_error/this->sequence_length / this->sub_iter < 0.01) {
	if (rand()%2 == 0) {
		cout << "CLEAR_INNER_STATE success" << endl;
		cout << "score: " << this->sum_error/this->sequence_length / this->sub_iter << endl;

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

		this->curr_num_states_cleared = this->test_num_states_cleared;

		this->clean_inner_state_index++;

		cout << "ending CLEAR_INNER_STATE" << endl;
		clean_transform_helper();
	} else {
		cout << "CLEAR_INNER_STATE fail" << endl;
		cout << "score: " << this->sum_error/this->sequence_length / this->sub_iter << endl;

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

		this->test_num_states_cleared = this->curr_num_states_cleared;

		this->clean_inner_step_index++;
		if (this->clean_inner_step_index >= this->sequence_length) {
			cout << "ending CLEAR_INNER_STATE" << endl;
			cout << "DONE" << endl;

			this->state = FOLD_STATE_DONE;
		} else {
			this->clean_inner_state_index = 0;

			cout << "ending CLEAR_INNER_STATE" << endl;
			clean_transform_helper();
		}
	}
}

void Fold::clear_inner_state_from_load() {
	this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
	this->test_state_not_needed_locally = this->curr_state_not_needed_locally;

	this->test_num_states_cleared[this->clean_inner_step_index]++;

	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_states;

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

	cout << "starting CLEAR_INNER_STATE " << this->clean_inner_step_index << " " << this->test_num_states_cleared[this->clean_inner_step_index] << endl;

	this->state = FOLD_STATE_CLEAR_INNER_STATE;
	this->state_iter = 0;
	this->sub_iter = 0;
	this->sum_error = 0.0;
}
