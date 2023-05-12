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

	if (misguess_improvement_t_value > 1.282) {	// >90%
		cout << "ADD_OUTER_STATE success" << endl;

		this->curr_num_new_outer_states = this->test_num_new_outer_states;

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
				it != this->curr_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
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
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
				it != this->curr_inner_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		// no change to this->curr_num_new_inner_states
		this->curr_state_networks = this->test_state_networks;
		this->test_state_networks.clear();
		this->curr_score_networks = this->test_score_networks;
		this->test_score_networks.clear();
		this->curr_inner_state_networks = this->test_inner_state_networks;
		this->test_inner_state_networks.clear();

		this->curr_branch_average_score = this->test_branch_average_score;
		this->test_branch_average_score = 0.0;
		this->curr_branch_existing_average_score = this->test_branch_existing_average_score;
		this->test_branch_existing_average_score = 0.0;
		this->curr_replace_average_score = this->test_replace_average_score;
		this->test_replace_average_score = 0.0;
		this->curr_replace_average_misguess = this->test_replace_average_misguess;
		this->test_replace_average_misguess = 0.0;
		this->curr_replace_misguess_variance = this->test_replace_misguess_variance;
		this->test_replace_misguess_variance = 0.0;

		this->experiment_added_state = true;
	} else {
		cout << "ADD_OUTER_STATE fail" << endl;

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

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_inner_state_networks.begin();
				it != this->test_inner_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		this->test_inner_state_networks.clear();
	}

	if (this->curr_replace_average_misguess > 0.01) {	// TODO: find systematic way to decide if further misguess improvement isn't worth it
		this->test_num_new_outer_states = this->curr_num_new_outer_states;
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
				it != this->curr_outer_state_networks.end(); it++) {
			this->test_outer_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				this->test_outer_state_networks[it->first].push_back(vector<StateNetwork*>());
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					this->test_outer_state_networks[it->first][n_index].push_back(
						new StateNetwork(it->second[n_index][s_index]));
				}
			}
		}

		this->test_starting_score_network = new StateNetwork(this->curr_starting_score_network);

		this->test_num_new_inner_states = this->curr_num_new_inner_states+1;
		int num_inner_networks = this->sum_inner_inputs
			+ this->curr_num_new_inner_states
			+ this->num_sequence_states;
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			this->test_state_networks.push_back(vector<StateNetwork*>());

			for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
				this->test_state_networks[f_index].push_back(new StateNetwork(
					this->curr_state_networks[f_index][i_index]));
				this->test_state_networks[f_index][i_index]->add_new_inner();
			}
			if (this->is_inner_scope[f_index]) {
				this->test_state_networks[f_index].push_back(new StateNetwork(0,
																			  this->num_sequence_states,
																			  this->sum_inner_inputs+this->test_num_new_inner_states,
																			  this->curr_num_new_outer_states,
																			  20));
			} else {
				this->test_state_networks[f_index].push_back(new StateNetwork(1,
																			  this->num_sequence_states,
																			  this->sum_inner_inputs+this->test_num_new_inner_states,
																			  this->curr_num_new_outer_states,
																			  20));
			}
			for (int s_index = this->sum_inner_inputs+this->curr_num_new_inner_states; s_index < num_inner_networks; s_index++) {
				this->test_state_networks[f_index].push_back(new StateNetwork(
					this->curr_state_networks[f_index][s_index]));
				this->test_state_networks[f_index][s_index+1]->add_new_inner();
			}

			this->test_score_networks.push_back(new StateNetwork(this->curr_score_networks[f_index]));
			this->test_score_networks[f_index]->add_new_inner();
		}

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
				it != this->curr_inner_state_networks.end(); it++) {
			Scope* inner_scope = solution->scopes[it->first];
			this->test_inner_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				this->test_inner_state_networks[it->first].push_back(vector<StateNetwork*>());
				if (it->second[n_index].size() != 0) {
					for (int s_index = 0; s_index < this->curr_num_new_inner_states; s_index++) {
						this->test_inner_state_networks[it->first][n_index].push_back(
							new StateNetwork(it->second[n_index][s_index]));
						this->test_inner_state_networks[it->first][n_index][s_index]->add_new_outer();
					}

					this->test_inner_state_networks[it->first][n_index].push_back(
						new StateNetwork(1,
										 inner_scope->num_states,
										 0,
										 this->test_num_new_inner_states,
										 20));
				}
			}
		}

		cout << "ending ADD_OUTER_STATE" << endl;
		cout << "starting ADD_INNER_STATE " << this->test_num_new_inner_states << endl;

		this->state = FOLD_STATE_ADD_INNER_STATE;
		this->state_iter = -1;
		this->sum_error = 0.0;
	} else {
		this->curr_inner_inputs_needed = vector<bool>(this->sum_inner_inputs, true);
		this->test_inner_inputs_needed = this->curr_inner_inputs_needed;

		int num_inner_networks = this->sum_inner_inputs
			+ this->curr_num_new_inner_states
			+ this->num_sequence_states;
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			this->curr_state_networks_not_needed.push_back(vector<bool>(num_inner_networks, false));
		}
		this->test_state_networks_not_needed = this->curr_state_networks_not_needed;

		if (this->sum_inner_inputs == 0) {
			cout << "ending ADD_OUTER_STATE" << endl;
			cout << "EXPERIMENT_DONE" << endl;

			this->state = FOLD_STATE_EXPERIMENT_DONE;
		} else {
			this->remove_inner_input_index = 0;
			this->test_inner_inputs_needed[this->remove_inner_input_index] = false;
			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				this->test_state_networks_not_needed[f_index][this->remove_inner_input_index] = true;
			}

			// outer unchanged

			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				// this->test_state_networks cleared above
				this->test_state_networks.push_back(vector<StateNetwork*>());

				for (int s_index = 0; s_index < num_inner_networks; s_index++) {
					if (!this->test_state_networks_not_needed[f_index][s_index]) {
						this->test_state_networks[f_index].push_back(new StateNetwork(this->curr_state_networks[f_index][s_index]));
					} else {
						this->test_state_networks[f_index].push_back(NULL);
					}
				}

				// this->test_score_networks cleared above
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

			cout << "ending ADD_OUTER_STATE" << endl;
			cout << "starting REMOVE_INNER_INPUT " << this->remove_inner_input_index << endl;

			this->state = FOLD_STATE_REMOVE_INNER_INPUT;
			this->state_iter = -1;
			this->sum_error = 0.0;
		}
	}
}
