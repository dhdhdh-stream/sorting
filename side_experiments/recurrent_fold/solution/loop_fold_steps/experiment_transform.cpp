#include "loop_fold.h"

#include <cmath>
#include <iostream>

#include "globals.h"

using namespace std;

void LoopFold::experiment_end() {
	double score_standard_deviation = sqrt(*this->existing_score_variance);
	double misguess_standard_deviation = sqrt(*this->existing_misguess_variance);

	double score_improvement = this->test_average_score - *this->existing_average_score;
	cout << "this->test_average_score: " << this->test_average_score << endl;
	cout << "this->existing_average_score: " << *this->existing_average_score << endl;

	double score_improvement_t_value = score_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "score_improvement_t_value: " << score_improvement_t_value << endl;

	double misguess_improvement = *this->existing_average_misguess - this->test_average_misguess;
	cout << "this->test_average_misguess: " << this->test_average_misguess << endl;
	cout << "this->existing_average_misguess: " << *this->existing_average_misguess << endl;

	double misguess_improvement_t_value = misguess_improvement
		/ (misguess_standard_deviation / sqrt(20000));
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	// bool is_success;
	// if (score_improvement_t_value > 2.326) {
	// 	cout << "EXPERIMENT success" << endl;
	// 	is_success = true;
	// } else if (*this->existing_average_misguess > 0.01
	// 		&& misguess_improvement_t_value > 2.326
	// 		&& score_improvement_t_value > -0.842) {
	// 	cout << "EXPERIMENT success" << endl;
	// 	is_success = true;
	// } else {
	// 	cout << "EXPERIMENT fail" << endl;
	// 	is_success = false;
	// }

	// if (is_success) {
	if (true) {
		this->curr_outer_state_networks = this->test_outer_state_networks;
		this->test_outer_state_networks.clear();

		this->curr_starting_state_networks = this->test_starting_state_networks;
		this->test_starting_state_networks.clear();

		this->curr_continue_score_network = this->test_continue_score_network;
		this->test_continue_score_network = NULL;
		this->curr_continue_misguess_network = this->test_continue_misguess_network;
		this->test_continue_misguess_network = NULL;
		this->curr_halt_score_network = this->test_halt_score_network;
		this->test_halt_score_network = NULL;
		this->curr_halt_misguess_network = this->test_halt_misguess_network;
		this->test_halt_misguess_network = NULL;

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

		// if (this->curr_average_misguess > 0.01) {	// TODO: find systematic way to decide if further misguess improvement isn't worth it
		if (true) {
			this->explore_added_state = false;

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

			this->test_num_new_inner_states = this->curr_num_new_inner_states;
			for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
				// this->test_starting_state_networks cleared above
				this->test_starting_state_networks.push_back(new StateNetwork(
					this->curr_starting_state_networks[i_index]));
				this->test_starting_state_networks[i_index]->add_new_outer();
			}

			this->test_continue_score_network = new StateNetwork(this->curr_continue_score_network);
			this->test_continue_score_network->add_new_outer();
			this->test_continue_misguess_network = new StateNetwork(this->curr_continue_misguess_network);
			this->test_continue_misguess_network->add_new_outer();
			this->test_halt_score_network = new StateNetwork(this->curr_halt_score_network);
			this->test_halt_score_network->add_new_outer();
			this->test_halt_misguess_network = new StateNetwork(this->curr_halt_misguess_network);
			this->test_halt_misguess_network->add_new_outer();

			int num_inner_networks = this->sum_inner_inputs
				+ this->curr_num_new_inner_states
				+ this->num_local_states
				+ this->num_input_states;
			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				// this->test_state_networks cleared above
				this->test_state_networks.push_back(vector<StateNetwork*>());

				for (int s_index = 0; s_index < num_inner_networks; s_index++) {
					this->test_state_networks[f_index].push_back(new StateNetwork(
						this->curr_state_networks[f_index][s_index]));
					this->test_state_networks[f_index][s_index]->add_new_outer();
				}

				// this->test_score_networks cleared above
				this->test_score_networks.push_back(new StateNetwork(this->curr_score_networks[f_index]));
				this->test_score_networks[f_index]->add_new_outer();
			}
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
					it != this->curr_inner_state_networks.end(); it++) {
				this->test_inner_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					this->test_inner_state_networks[it->first].push_back(vector<StateNetwork*>());
					if (it->second[n_index].size() != 0) {
						// this->curr_num_new_inner_states = 1
						this->test_inner_state_networks[it->first][n_index].push_back(
							new StateNetwork(it->second[n_index][0]));
					}
				}
			}

			cout << "starting ADD_OUTER_STATE " << this->test_num_new_outer_states << endl;

			this->state = LOOP_FOLD_STATE_ADD_OUTER_STATE;
			this->sub_state = LOOP_FOLD_SUB_STATE_LEARN;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
			cout << "EXPERIMENT_DONE" << endl;

			this->state = LOOP_FOLD_STATE_EXPERIMENT_DONE;
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

		for (int i_index = 0; i_index < (int)this->test_starting_state_networks.size(); i_index++) {
			delete this->test_starting_state_networks[i_index];
		}

		delete this->test_continue_score_network;
		delete this->test_continue_misguess_network;
		delete this->test_halt_score_network;
		delete this->test_halt_misguess_network;

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->test_state_networks[f_index].size(); s_index++) {
				delete this->test_state_networks[f_index][s_index];
			}

			delete this->test_score_networks[f_index];
		}

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_inner_state_networks.begin();
				it != this->test_inner_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		this->test_inner_state_networks.clear();

		this->state = LOOP_FOLD_STATE_EXPERIMENT_FAIL;
	}
}
