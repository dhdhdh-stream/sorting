#include "fold.h"

#include <cmath>
#include <iostream>

#include "globals.h"

using namespace std;

void Fold::add_outer_state_end() {
	// could lead to change in scores, but don't worry about for now

	cout << "this->curr_replace_average_misguess: " << this->curr_replace_average_misguess << endl;
	cout << "this->test_replace_average_misguess: " << this->test_replace_average_misguess << endl;

	double misguess_improvement = this->curr_replace_average_misguess - this->test_replace_average_misguess;
	cout << "misguess_improvement: " << misguess_improvement << endl;

	// heuristic for "impactful" improvement
	// replace_misguess_variance may be higher if score increased, but misguess also increased

	double existing_misguess_standard_deviation = sqrt(*this->existing_misguess_variance);
	double new_misguess_standard_deviation = sqrt(this->curr_replace_misguess_variance);

	double misguess_improvement_t_value;
	if (existing_misguess_standard_deviation > new_misguess_standard_deviation) {
		misguess_improvement_t_value = misguess_improvement
			/ (existing_misguess_standard_deviation / sqrt(20000));
	} else {
		misguess_improvement_t_value = misguess_improvement
			/ (new_misguess_standard_deviation / sqrt(20000));
	}
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	if (misguess_improvement_t_value > 0.842) {	// >80%
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
				it != this->curr_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		this->curr_num_new_outer_states = this->test_num_new_outer_states;
		this->curr_outer_state_networks = this->test_outer_state_networks;
		this->test_outer_state_networks.clear();

		delete this->curr_starting_score_network;
		this->curr_starting_score_network = this->test_starting_score_network;
		this->test_starting_score_network = NULL;

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->curr_state_networks[f_index].size(); s_index++) {
				delete this->curr_state_networks[f_index][s_index];
			}

			delete this->curr_score_networks[f_index];
		}
		// no change to this->curr_num_new_inner_states
		this->curr_state_networks = this->test_state_networks;
		this->curr_score_networks = this->test_score_networks;

		this->curr_branch_average_score = this->test_branch_average_score;
		this->test_branch_average_score = 0.0;
		this->curr_existing_average_improvement = this->test_existing_average_improvement;
		this->test_existing_average_improvement = 0.0;
		this->curr_replace_average_score = this->test_replace_average_score;
		this->test_replace_average_score = 0.0;
		this->curr_replace_average_misguess = this->test_replace_average_misguess;
		this->test_replace_average_misguess = 0.0;
		this->curr_replace_misguess_variance = this->test_replace_misguess_variance;
		this->test_replace_misguess_variance = 0.0;

		if (this->curr_replace_average_misguess > 0.01) {	// TODO: find systematic way to decide if further misguess improvement isn't worth it
			this->test_num_new_outer_states = this->curr_num_new_outer_states+1;
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
					it != this->curr_outer_state_networks.end(); it++) {
				Scope* outer_scope = solution->scopes[it->first];
				this->test_outer_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					this->test_outer_state_networks[it->first].push_back(vector<StateNetwork*>());
					if (it->second[n_index].size() != 0) {
						for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
							this->test_outer_state_networks[it->first][n_index].push_back(
								new StateNetwork(it->second[n_index][s_index]));
							this->test_outer_state_networks[it->first][n_index][s_index]->add_new_outer();
						}

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

			// no change to num_new_inner_states
			int curr_total_num_states = this->sum_inner_inputs
				+ this->curr_num_new_inner_states
				+ this->num_sequence_local_states
				+ this->num_sequence_input_states
				+ this->curr_num_new_outer_states;
			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				for (int s_index = 0; s_index < curr_total_num_states; s_index++) {
					this->test_state_networks[f_index][s_index] = new StateNetwork(
						this->curr_state_networks[f_index][s_index]);
					this->test_state_networks[f_index][s_index]->add_new_outer();
				}

				if (this->is_inner_scope[f_index]) {
					this->test_state_networks[f_index].push_back(new StateNetwork(0,
																				  this->num_sequence_local_states,
																				  this->num_sequence_input_states,
																				  this->sum_inner_inputs+this->curr_num_new_inner_states,
																				  this->test_num_new_outer_states,
																				  20));
				} else {
					this->test_state_networks[f_index].push_back(new StateNetwork(1,
																				  this->num_sequence_local_states,
																				  this->num_sequence_input_states,
																				  this->sum_inner_inputs+this->curr_num_new_inner_states,
																				  this->test_num_new_outer_states,
																				  20));
				}

				this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
				this->test_score_networks[f_index]->add_new_outer();
			}

			cout << "ending ADD_OUTER_STATE" << endl;
			cout << "starting ADD_OUTER_STATE " << this->test_num_new_outer_states << endl;

			this->state = FOLD_STATE_ADD_OUTER_STATE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
			// clear for add_to_clean
			this->test_state_networks.clear();
			this->test_score_networks.clear();

			cout << "EXPLORE_DONE" << endl;

			this->state = FOLD_STATE_EXPLORE_DONE;
		}
	} else {
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.begin();
				it != this->test_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		this->test_outer_state_networks.clear();

		delete this->test_starting_score_network;
		this->test_starting_score_network = NULL;

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->test_state_networks[f_index].size(); s_index++) {
				delete this->test_state_networks[f_index][s_index];
			}

			delete this->test_score_networks[f_index];
		}
		this->test_state_networks.clear();
		this->test_score_networks.clear();

		cout << "ending ADD_OUTER_STATE" << endl;
		cout << "EXPLORE_DONE" << endl;

		this->state = FOLD_STATE_EXPLORE_DONE;
	}
}
