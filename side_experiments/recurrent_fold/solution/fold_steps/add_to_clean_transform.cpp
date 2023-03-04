#include "fold.h"

using namespace std;

void Fold::add_to_clean() {
	if (this->curr_outer_state_networks.size() == 0) {
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

		// test_state_networks/test_score_networks already sized
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
	} else {
		this->clean_outer_scope_index = 0;
		map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
		int clean_outer_scope_scope_id = it->first;

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
				it != this->curr_outer_state_networks.end(); it++) {
			if (it->first != clean_outer_scope_scope_id) {
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
		// test_state_networks/test_score_networks already sized
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
	}
}
