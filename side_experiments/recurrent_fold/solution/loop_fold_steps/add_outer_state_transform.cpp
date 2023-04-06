#include "loop_fold.h"

#include <cmath>
#include <iostream>

#include "globals.h"

using namespace std;

void LoopFold::add_outer_state_end() {
	// could lead to change in scores, but don't worry about for now

	cout << "this->curr_average_misguess: " << this->curr_average_misguess << endl;
	cout << "this->test_average_misguess: " << this->test_average_misguess << endl;

	double misguess_improvement = this->curr_average_misguess - this->test_average_misguess;
	cout << "misguess_improvement: " << misguess_improvement << endl;

	// heuristic for "impactful" improvement
	// replace_misguess_variance may be higher if score increased, but misguess also increased

	double existing_misguess_standard_deviation = sqrt(*this->existing_misguess_variance);
	double new_misguess_standard_deviation = sqrt(this->curr_misguess_variance);

	double misguess_improvement_t_value;
	if (existing_misguess_standard_deviation > new_misguess_standard_deviation) {
		misguess_improvement_t_value = misguess_improvement
			/ (existing_misguess_standard_deviation / sqrt(20000));
	} else {
		misguess_improvement_t_value = misguess_improvement
			/ (new_misguess_standard_deviation / sqrt(20000));
	}
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	// if (misguess_improvement_t_value > 1.282) {	// >90%
	if (rand()%2) {
		cout << "ADD_OUTER_STATE success" << endl;
		cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

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

		for (int i_index = 0; i_index < (int)this->curr_starting_state_networks.size(); i_index++) {
			delete this->curr_starting_state_networks[i_index];
		}
		this->curr_starting_state_networks = this->test_starting_state_networks;
		this->test_starting_state_networks.clear();

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

		// no change to this->curr_num_new_inner_states

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
		this->curr_state_networks = this->test_state_networks;
		this->test_state_networks.clear();
		this->curr_score_networks = this->test_score_networks;
		this->test_score_networks.clear();
		this->curr_inner_state_networks = this->test_inner_state_networks;
		this->test_inner_state_networks.clear();

		this->curr_average_score = this->test_average_score;
		this->test_average_score = 0.0;
		this->curr_score_variance = this->test_score_variance;
		this->test_score_variance = 0.0;
		this->curr_average_misguess = this->test_average_misguess;
		this->test_average_misguess = 0.0;
		this->curr_misguess_variance = this->test_misguess_variance;
		this->test_misguess_variance = 0.0;

		this->explore_added_state = true;
	} else {
		cout << "ADD_OUTER_STATE fail" << endl;
		cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.begin();
				it != this->test_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		this->test_outer_state_networks.clear();

		for (int i_index = 0; i_index < (int)this->test_starting_state_networks.size(); i_index++) {
			delete this->test_starting_state_networks[i_index];
		}
		this->test_starting_state_networks.clear();

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

	// if (this->curr_average_misguess > 0.01) {	// TODO: find systematic way to decide if further misguess improvement isn't worth it
	if (true) {
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

		this->test_num_new_inner_states = this->curr_num_new_inner_states+1;
		for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
			// this->test_starting_state_networks cleared above
			this->test_starting_state_networks.push_back(new StateNetwork(
				this->curr_starting_state_networks[i_index]));
			this->test_starting_state_networks[i_index]->add_new_inner();
		}
		this->test_starting_state_networks.push_back(new StateNetwork(0,
																	  this->num_local_states,
																	  this->num_input_states,
																	  this->sum_inner_inputs+this->test_num_new_inner_states,
																	  this->curr_num_new_outer_states,
																	  20));

		this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
		this->test_continue_score_network->add_new_inner();
		this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
		this->test_continue_misguess_network->add_new_inner();
		this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
		this->test_halt_score_network->add_new_inner();
		this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);
		this->test_halt_misguess_network->add_new_inner();

		int num_inner_networks = this->sum_inner_inputs
			+ this->curr_num_new_inner_states
			+ this->num_local_states
			+ this->num_input_states;
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			this->test_state_networks.push_back(vector<StateNetwork*>());

			for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
				this->test_state_networks[f_index].push_back(new StateNetwork(
					this->curr_state_networks[f_index][i_index]));
				this->test_state_networks[f_index][i_index]->add_new_inner();
			}
			if (this->is_inner_scope[f_index]) {
				this->test_state_networks[f_index].push_back(new StateNetwork(0,
																			  this->num_local_states,
																			  this->num_input_states,
																			  this->sum_inner_inputs+this->test_num_new_inner_states,
																			  this->curr_num_new_outer_states,
																			  20));
			} else {
				this->test_state_networks[f_index].push_back(new StateNetwork(1,
																			  this->num_local_states,
																			  this->num_input_states,
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
										 inner_scope->num_local_states,
										 inner_scope->num_input_states,
										 0,
										 this->test_num_new_inner_states,
										 20));
				}
			}
		}

		cout << "ending ADD_OUTER_STATE" << endl;
		cout << "starting ADD_INNER_STATE " << this->test_num_new_inner_states << endl;

		this->state = LOOP_FOLD_STATE_ADD_INNER_STATE;
		this->sub_state = LOOP_FOLD_SUB_STATE_LEARN;
		this->state_iter = 0;
		this->sum_error = 0.0;
	} else {
		cout << "ending ADD_OUTER_STATE" << endl;
		cout << "EXPERIMENT_DONE" << endl;

		this->state = LOOP_FOLD_STATE_EXPERIMENT_DONE;
	}
}
