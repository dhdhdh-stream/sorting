#include "loop_fold.h"

#include <iostream>

using namespace std;

void LoopFold::experiment_to_clean() {
	// state_networks_not_needed already initialized

	this->curr_outer_scopes_needed.insert(this->scope_context[0]);
	for (int c_index = 1; c_index < (int)this->scope_context.size(); c_index++) {
		this->curr_outer_scopes_needed.insert(this->scope_context[c_index]);
		this->curr_outer_contexts_needed.insert(make_pair(this->scope_context[c_index], this->node_context[c_index]));
	}
	this->reverse_test_outer_scopes_needed = this->curr_outer_scopes_needed;
	this->reverse_test_outer_contexts_needed = this->curr_outer_contexts_needed;

	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_states;

	// Note: don't adjust context even if outer state not needed, as trained within specific context
	if (this->curr_outer_state_networks.size() == 0) {
		this->curr_num_new_outer_states = 0;
		for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
			if (i_index >= this->sum_inner_inputs
					|| this->curr_inner_inputs_needed[i_index]) {
				this->curr_starting_state_networks[i_index]->remove_new_outer();
			}
		}
		this->curr_continue_score_network->remove_new_outer();
		this->curr_continue_misguess_network->remove_new_outer();
		this->curr_halt_score_network->remove_new_outer();
		this->curr_halt_misguess_network->remove_new_outer();
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < num_inner_networks; s_index++) {
				if (!this->curr_state_networks_not_needed[f_index][s_index]) {
					this->curr_state_networks[f_index][s_index]->remove_new_outer();
				}
			}

			this->curr_score_networks[f_index]->remove_new_outer();
		}
		// this->curr_inner_state_networks unchanged

		if (this->curr_inner_state_networks.size() == 0) {
			// initialize inner clean
			this->clean_inner_step_index = 0;
			this->clean_inner_state_index = 0;
			this->state = LOOP_FOLD_STATE_REMOVE_INNER_NETWORK;

			// initialize size
			this->test_starting_state_networks = this->curr_starting_state_networks;
			this->test_state_networks = this->curr_state_networks;
			this->test_score_networks = this->curr_score_networks;

			remove_inner_network_transform_helper();
		} else {
			this->clean_inner_scope_index = 0;
			map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
			int clean_inner_scope_scope_id = it->first;

			this->reverse_test_inner_scopes_needed.insert(clean_inner_scope_scope_id);

			for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
				// this->test_starting_state_networks previously cleared
				if (i_index >= this->sum_inner_inputs
						|| this->curr_inner_inputs_needed[i_index]) {
					this->test_starting_state_networks.push_back(new StateNetwork(
						this->curr_starting_state_networks[i_index]));
				} else {
					this->test_starting_state_networks.push_back(NULL);
				}
			}

			this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
			this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
			this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
			this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);

			// test_state_networks/test_score_networks previously cleared
			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				this->test_state_networks.push_back(vector<StateNetwork*>());
				for (int s_index = 0; s_index < num_inner_networks; s_index++) {
					if (!this->curr_state_networks_not_needed[f_index][s_index]) {
						this->test_state_networks[f_index].push_back(new StateNetwork(
							this->curr_state_networks[f_index][s_index]));
					} else {
						this->test_state_networks[f_index].push_back(NULL);
					}
				}

				this->test_score_networks.push_back(new StateNetwork(this->curr_score_networks[f_index]));
			}

			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
					it != this->curr_inner_state_networks.end(); it++) {
				if (it->first != clean_inner_scope_scope_id) {
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

		for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
			// this->test_starting_state_networks previously cleared
			if (i_index >= this->sum_inner_inputs
					|| this->curr_inner_inputs_needed[i_index]) {
				this->test_starting_state_networks.push_back(new StateNetwork(
					this->curr_starting_state_networks[i_index]));
			} else {
				this->test_starting_state_networks.push_back(NULL);
			}
		}

		this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
		this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
		this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
		this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);

		// test_state_networks/test_score_networks previously cleared
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			this->test_state_networks.push_back(vector<StateNetwork*>());
			for (int s_index = 0; s_index < num_inner_networks; s_index++) {
				if (!this->curr_state_networks_not_needed[f_index][s_index]) {
					this->test_state_networks[f_index].push_back(new StateNetwork(
						this->curr_state_networks[f_index][s_index]));
				} else {
					this->test_state_networks[f_index].push_back(NULL);
				}
			}

			this->test_score_networks.push_back(new StateNetwork(this->curr_score_networks[f_index]));
		}

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
				it != this->curr_inner_state_networks.end(); it++) {
			this->test_inner_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				this->test_inner_state_networks[it->first].push_back(vector<StateNetwork*>());
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					this->test_inner_state_networks[it->first][n_index].push_back(
						new StateNetwork(it->second[n_index][s_index]));
				}
			}
		}

		cout << "starting REMOVE_OUTER_SCOPE " << this->clean_outer_scope_index << endl;

		this->state = LOOP_FOLD_STATE_REMOVE_OUTER_SCOPE;
		this->state_iter = -1;
		this->sub_iter = 0;
		this->sum_error = 0.0;
	}
}
