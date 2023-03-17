#include "fold.h"

#include <iostream>

using namespace std;

void Fold::add_to_clean() {
	// initialize clean
	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_local_states
		+ this->num_sequence_input_states;
	int total_num_states = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_local_states
		+ this->num_sequence_input_states
		+ this->curr_num_new_outer_states;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		this->curr_state_networks_not_needed.push_back(vector<bool>(num_inner_networks, false));
		this->curr_state_not_needed_locally.push_back(vector<bool>(total_num_states, false));
		this->curr_num_states_cleared.push_back(0);

		this->test_state_networks_not_needed.push_back(vector<bool>(num_inner_networks, false));
		this->test_state_not_needed_locally.push_back(vector<bool>(total_num_states, false));
		this->test_num_states_cleared.push_back(0);
	}

	if (this->curr_outer_state_networks.size() == 0) {
		this->curr_num_new_outer_states = 0;
		this->curr_starting_score_network->remove_new_outer();
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < num_inner_networks; s_index++) {
				this->curr_state_networks[f_index][s_index]->remove_new_outer();
			}

			this->curr_score_networks[f_index]->remove_new_outer();
		}

		// initialize inner clean
		this->clean_inner_step_index = 0;
		this->clean_inner_state_index = 0;

		this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
		this->test_state_not_needed_locally = this->curr_state_not_needed_locally;
		this->test_num_states_cleared = this->curr_num_states_cleared;

		this->test_state_networks_not_needed[0][0] = true;

		// test_state_networks/test_score_networks previously cleared
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			this->test_state_networks.push_back(vector<StateNetwork*>());
			for (int s_index = 0; s_index < num_inner_networks; s_index++) {
				if (!this->test_state_networks_not_needed[f_index][s_index]) {
					this->test_state_networks[f_index].push_back(new StateNetwork(this->curr_state_networks[f_index][s_index]));
				} else {
					this->test_state_networks[f_index].push_back(NULL);
				}
			}

			this->test_score_networks.push_back(new StateNetwork(this->curr_score_networks[f_index]));
		}

		cout << "starting REMOVE_INNER_NETWORK " << this->clean_inner_step_index << " " << this->clean_inner_state_index << endl;

		this->state = FOLD_STATE_REMOVE_INNER_NETWORK;
		this->state_iter = 0;
		this->sub_state_iter = 0;
		this->sum_error = 0.0;
	} else {
		this->clean_outer_scope_index = 0;
		map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
		int clean_outer_scope_scope_id = it->first;

		this->reverse_test_outer_scopes_needed.insert(clean_outer_scope_scope_id);

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
				it != this->curr_outer_state_networks.end(); it++) {
			if (it->first != clean_outer_scope_scope_id) {
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

		// test_state_networks/test_score_networks previously cleared
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			this->test_state_networks.push_back(vector<StateNetwork*>());
			for (int s_index = 0; s_index < num_inner_networks; s_index++) {
				this->test_state_networks[f_index].push_back(new StateNetwork(
					this->curr_state_networks[f_index][s_index]));
			}

			this->test_score_networks.push_back(new StateNetwork(this->curr_score_networks[f_index]));
		}

		cout << "starting REMOVE_OUTER_SCOPE " << this->clean_outer_scope_index << endl;

		this->state = FOLD_STATE_REMOVE_OUTER_SCOPE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
		this->sum_error = 0.0;
	}
}
