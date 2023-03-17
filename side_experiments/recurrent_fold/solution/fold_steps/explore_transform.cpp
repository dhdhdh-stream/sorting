#include "fold.h"

#include <cmath>
#include <iostream>

#include "globals.h"

using namespace std;

void Fold::explore_end() {
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

	if (branch_improvement_t_value > 2.326) {	// >99%
		if (existing_improvement_t_value < 0.842	// 80%<
				&& this->is_recursive == 0) {
			cout << "FOLD_RESULT_REPLACE" << endl;
			this->explore_result = FOLD_RESULT_REPLACE;
		} else {
			cout << "FOLD_RESULT_BRANCH" << endl;
			this->explore_result = FOLD_RESULT_BRANCH;
		}
	} else if (misguess_improvement_t_value > 2.326	// >99%
			&& replace_improvement_t_value > -0.842	// 80%<
			&& this->is_recursive == 0) {
		cout << "FOLD_RESULT_REPLACE" << endl;
		this->explore_result = FOLD_RESULT_REPLACE;
	} else if (this->sequence_length < this->existing_sequence_length
			&& replace_improvement_t_value > 0.0
			&& misguess_improvement_t_value > 0.0
			&& this->is_recursive == 0) {
		cout << "FOLD_RESULT_REPLACE" << endl;
		this->explore_result = FOLD_RESULT_REPLACE;
	} else {
		cout << "FOLD_RESULT_FAIL" << endl;
		this->explore_result = FOLD_RESULT_FAIL;
	}

	if (this->explore_result != FOLD_RESULT_FAIL) {
		this->curr_outer_state_networks = this->test_outer_state_networks;
		this->test_outer_state_networks.clear();

		this->curr_starting_score_network = this->test_starting_score_network;
		this->test_starting_score_network = NULL;

		this->curr_state_networks = this->test_state_networks;
		this->test_state_networks.clear();
		this->curr_score_networks = this->test_score_networks;
		this->test_score_networks.clear();

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

			this->test_starting_score_network = new StateNetwork(this->curr_starting_score_network);
			this->test_starting_score_network->add_new_outer();

			this->test_num_new_inner_states = this->curr_num_new_inner_states;
			int num_inner_networks = this->sum_inner_inputs
				+ this->curr_num_new_inner_states
				+ this->num_sequence_local_states
				+ this->num_sequence_input_states;
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

			cout << "starting ADD_OUTER_STATE " << this->test_num_new_outer_states << endl;

			this->state = FOLD_STATE_ADD_OUTER_STATE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
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

		this->state = FOLD_STATE_EXPLORE_FAIL;
	}
}
