#include "fold.h"

#include <cmath>
#include <iostream>

#include "globals.h"

using namespace std;

void Fold::experiment_end() {
	double score_standard_deviation = sqrt(*this->existing_score_variance);
	double misguess_standard_deviation = sqrt(*this->existing_misguess_variance);

	cout << "this->existing_average_score: " << *this->existing_average_score << endl;

	double branch_improvement = this->test_branch_average_score - *this->existing_average_score;
	cout << "this->test_branch_average_score: " << this->test_branch_average_score << endl;

	cout << "this->test_existing_average_improvement: " << this->test_existing_average_improvement << endl;

	double replace_improvement = this->test_replace_average_score - *this->existing_average_score;
	cout << "this->test_replace_average_score: " << this->test_replace_average_score << endl;

	double misguess_improvement = *this->existing_average_misguess - this->test_replace_average_misguess;
	cout << "this->test_replace_average_misguess: " << this->test_replace_average_misguess << endl;
	cout << "this->existing_average_misguess: " << *this->existing_average_misguess << endl;

	// 0.0001 rolling average variance approx. equal to 20000 average variance (?)

	double branch_improvement_t_value = branch_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "branch_improvement_t_value: " << branch_improvement_t_value << endl;

	double existing_improvement_t_value = this->test_existing_average_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "existing_improvement_t_value: " << existing_improvement_t_value << endl;

	double replace_improvement_t_value = replace_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "replace_improvement_t_value: " << replace_improvement_t_value << endl;

	double misguess_improvement_t_value = misguess_improvement
		/ (misguess_standard_deviation / sqrt(20000));
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	// if (branch_improvement_t_value > 2.326) {	// >99%
	if (rand()%3 != 0) {	// >99%
		// if (existing_improvement_t_value < 0.842	// 80%<
		// 		&& this->is_recursive == 0) {
		if (rand()%2 == 0) {
			cout << "FOLD_RESULT_REPLACE" << endl;
			this->experiment_result = FOLD_RESULT_REPLACE;
		} else {
			cout << "FOLD_RESULT_BRANCH" << endl;
			this->experiment_result = FOLD_RESULT_BRANCH;
		}
	// } else if (*this->existing_average_misguess > 0.01
	// 		&& misguess_improvement_t_value > 2.326	// >99%
	// 		&& replace_improvement_t_value > -0.842	// 80%<
	// 		&& this->is_recursive == 0) {
	} else if (false) {
		cout << "FOLD_RESULT_REPLACE" << endl;
		this->experiment_result = FOLD_RESULT_REPLACE;
	} else {
		cout << "FOLD_RESULT_FAIL" << endl;
		this->experiment_result = FOLD_RESULT_FAIL;
	}

	if (this->experiment_result != FOLD_RESULT_FAIL) {
		this->curr_outer_state_networks = this->test_outer_state_networks;
		this->test_outer_state_networks.clear();

		this->curr_starting_score_network = this->test_starting_score_network;
		this->test_starting_score_network = NULL;

		this->curr_state_networks = this->test_state_networks;
		this->test_state_networks.clear();
		this->curr_score_networks = this->test_score_networks;
		this->test_score_networks.clear();
		this->curr_inner_state_networks = this->test_inner_state_networks;
		this->test_inner_state_networks.clear();

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

		// if (this->curr_replace_average_misguess > 0.01) {	// TODO: find systematic way to decide if further misguess improvement isn't worth it
		if (true) {
			this->experiment_added_state = false;

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
											 outer_scope->num_states,
											 0,
											 this->test_num_new_outer_states,
											 20));
					}
				}
			}

			this->test_starting_score_network = new StateNetwork(this->curr_starting_score_network);
			this->test_starting_score_network->add_new_outer();

			this->test_num_new_inner_states = this->curr_num_new_inner_states;
			int num_inner_networks = this->sum_inner_inputs
				+ this->curr_num_new_inner_states
				+ this->num_sequence_states;
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
						for (int s_index = 0; s_index < this->curr_num_new_inner_states; s_index++) {
							this->test_inner_state_networks[it->first][n_index].push_back(
								new StateNetwork(it->second[n_index][s_index]));
						}
					}
				}
			}

			cout << "starting ADD_OUTER_STATE " << this->test_num_new_outer_states << endl;

			this->state = FOLD_STATE_ADD_OUTER_STATE;
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
					this->test_score_networks[f_index]->add_new_outer();
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

				cout << "starting REMOVE_INNER_INPUT " << this->remove_inner_input_index << endl;

				this->state = FOLD_STATE_REMOVE_INNER_INPUT;
				this->state_iter = -1;
				this->sum_error = 0.0;
			}
		}
	} else {
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_inner_scope[f_index]) {
				delete this->inner_scope_scale_mods[f_index];
			}
		}

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

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_inner_state_networks.begin();
				it != this->test_inner_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}
		this->test_inner_state_networks.clear();

		this->state = FOLD_STATE_EXPERIMENT_FAIL;
	}
}
