#include "fold.h"

using namespace std;

void Fold::add_inner_state_end() {
	// TODO: check for score increase or misguess improvement
	if (/* SUCCESS */) {
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->curr_state_networks[f_index].size(); s_index++) {
				delete this->curr_state_networks[f_index][s_index];
			}

			delete this->curr_score_networks[f_index];
		}
		this->curr_num_new_inner_states = this->test_num_new_inner_states;
		this->curr_state_networks = this->test_state_networks;
		this->curr_score_networks = this->test_score_networks;

		// don't special case outer

		this->test_num_new_inner_states = this->curr_num_new_inner_states+1;
		int curr_total_num_states = this->sum_inner_inputs
			+ this->curr_num_new_inner_states
			+ this->num_local_states
			+ this->num_input_states
			+ this->curr_num_new_outer_states;
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
				this->test_state_networks[f_index][i_index] = new StateNetwork(
					this->curr_state_networks[f_index][i_index]);
				this->test_state_networks[f_index][i_index]->add_new_inner();
			}
			if (this->is_inner_scope[f_index]) {
				this->test_state_networks[f_index].insert(
					this->test_state_networks[f_index].begin()+this->sum_inner_inputs+this->curr_num_new_inner_states,
					new StateNetwork(0,
									 this->num_local_states,
									 this->num_input_states,
									 this->sum_inner_inputs+this->test_num_new_inner_states,
									 this->curr_num_new_outer_states,
									 20));
			} else {
				this->test_state_networks[f_index].insert(
					this->test_state_networks[f_index].begin()+this->sum_inner_inputs+this->curr_num_new_inner_states,
					new StateNetwork(1,
									 this->num_local_states,
									 this->num_input_states,
									 this->sum_inner_inputs+this->test_num_new_inner_states,
									 this->curr_num_new_outer_states,
									 20));
			}
			for (int s_index = this->sum_inner_inputs+this->curr_num_new_inner_states; s_index < curr_total_num_states; s_index++) {
				this->test_state_networks[f_index][s_index+1] = new StateNetwork(
					this->curr_state_networks[f_index][s_index]);
				this->test_state_networks[f_index][s_index+1]->add_new_inner();
			}

			this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
			this->test_score_networks[f_index]->add_new_inner();
		}

		this->state = STATE_ADD_INNER_STATE;
		this->state_iter = 0;
		this->sum_error = 0.0;
	} else {
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->test_state_networks[f_index].size(); s_index++) {
				delete this->test_state_networks[f_index][s_index];
			}

			delete this->test_score_networks[f_index];
		}
		this->test_state_networks.clear();
		this->test_score_networks.clear();

		this->test_num_new_outer_states = this->curr_num_new_outer_states+1;
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
				it != this->curr_outer_state_networks.end(); it++) {
			Scope* outer_scope = solution->scopes[it->first];
			this->test_outer_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				this->test_outer_state_networks[it->first].push_back(vector<StateNetwork*>());
				if (it->second[n_index].size() != 0) {
					// this->curr_num_new_outer_states = 1
					this->test_outer_state_networks[it->first][n_index].push_back(
						new StateNetwork(it->second[n_index][0]));
					this->test_outer_state_networks[it->first][n_index][0]->add_new_outer();
					// state networks track their needs, so robust against scope updates

					this->test_outer_state_networks[it->first][n_index].push_back(
						new StateNetwork(1,
										 outer_scope->num_local_states,
										 outer_scope->num_input_states,
										 0,
										 this->test_num_new_outer_states,
										 20));
				}
			}
		}

		this->test_starting_score_network = new StateNetwork(this->curr_starting_score_network);
		this->test_starting_score_network->add_new_outer();

		this->test_num_new_inner_states = this->curr_num_new_inner_states;
		int curr_total_num_states = this->sum_inner_inputs
			+ this->curr_num_new_inner_states
			+ this->num_local_states
			+ this->num_input_states
			+ this->curr_num_new_outer_states;
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < curr_total_num_states; s_index++) {
				// this->test_state_networks cleared above
				this->test_state_networks[f_index].push_back(new StateNetwork(
					this->curr_state_networks[f_index][s_index]));
				this->test_state_networks[f_index][s_index]->add_new_outer();
			}

			if (this->is_inner_scope[f_index]) {
				this->test_state_networks[f_index].push_back(new StateNetwork(0,
																			  this->num_local_states,
																			  this->num_input_states,
																			  this->sum_inner_inputs+this->curr_num_new_inner_states,
																			  this->test_num_new_outer_states,
																			  20));
			} else {
				this->test_state_networks[f_index].push_back(new StateNetwork(1,
																			  this->num_local_states,
																			  this->num_input_states,
																			  this->sum_inner_inputs+this->curr_num_new_inner_states,
																			  this->test_num_new_outer_states,
																			  20));
			}

			// this->test_score_networks cleared above
			this->test_score_networks.push_back(new StateNetwork(this->curr_score_networks[f_index]));
			this->test_score_networks[f_index]->add_new_outer();
		}

		this->state = STATE_ADD_OUTER_STATE;
		this->state_iter = 0;
		this->sum_error = 0.0;
	}
}
