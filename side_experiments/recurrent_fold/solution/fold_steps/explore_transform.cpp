#include "fold.h"

using namespace std;

void Fold::explore_end() {
	double score_standard_deviation = sqrt(*this->existing_score_variance);
	double misguess_standard_deviation = sqrt(*this->existing_misguess_variance);

	double replace_improvement = this->test_average_score - *this->existing_average_score;
	cout << "this->test_average_score: " << this->test_average_score << endl;
	cout << "this->existing_average_score: " << *this->existing_average_score << endl;

	double misguess_improvement = *this->existing_average_misguess - this->test_average_misguess;
	cout << "this->test_average_misguess: " << this->test_average_misguess << endl;
	cout << "this->existing_average_misguess: " << *this->existing_average_misguess << endl;

	// 0.0001 rolling average variance approx. equal to 20000 average variance (?)

	double replace_improvement_t_value = replace_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "replace_improvement_t_value: " << replace_improvement_t_value << endl;

	double misguess_improvement_t_value = misguess_improvement
		/ (misguess_standard_deviation / sqrt(20000));
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	cout << "this->new_noticably_better: " << this->new_noticably_better << endl;
	cout << "this->existing_noticably_better: " << this->existing_noticably_better << endl;

	if (this->new_noticably_better > 0) {
		if ((replace_improvement > 0.0 || abs(replace_improvement_t_value) < 0.842)	// 80%<
				&& this->existing_noticably_better == 0
				&& this->is_recursive == 0) {
			cout << "FOLD_RESULT_REPLACE" << endl;
			this->explore_result = FOLD_RESULT_REPLACE;
		} else {
			cout << "FOLD_RESULT_BRANCH" << endl;
			this->explore_result = FOLD_RESULT_BRANCH;
		}
	} else if ((replace_improvement_t_value > 0.0 || abs(replace_improvement_t_value) < 0.842)	// 80%<
			&& this->existing_noticably_better == 0
			&& this->is_recursive == 0) {
		if (misguess_improvement_t_value > 2.326) {	// >99%
			cout << "FOLD_RESULT_REPLACE" << endl;
			this->explore_result = FOLD_RESULT_REPLACE;
		} else if (this->sequence_length < this->existing_sequence_length
				&& misguess_improvement >= 0.0) {	// stricter condition for when replacing based on sequence length
			cout << "FOLD_RESULT_REPLACE" << endl;
			this->explore_result = FOLD_RESULT_REPLACE;
		} else {
			cout << "FOLD_RESULT_FAIL" << endl;
			this->explore_result = FOLD_RESULT_FAIL;
		}
	} else {
		cout << "FOLD_RESULT_FAIL" << endl;
		this->explore_result = FOLD_RESULT_FAIL;
	}

	if (this->explore_result != FOLD_RESULT_FAIL) {
		this->curr_outer_state_networks = this->test_outer_state_networks;
		this->curr_starting_score_network = this->test_starting_score_network;

		this->curr_state_networks = this->test_state_networks;
		this->curr_score_networks = this->test_score_networks;

		this->curr_average_score = this->test_average_score;
		this->curr_score_variance = this->test_score_variance;
		this->curr_average_misguess = this->test_average_misguess;
		this->curr_misguess_variance = this->test_misguess_variance;

		// no change to num_new_outer_states
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
				it != this->curr_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				this->test_outer_state_networks.insert({it->first, vector<vector<StateNetwork*>>()});
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					this->test_outer_state_networks[it->first][n_index].push_back(
						new StateNetwork(it->second[n_index][s_index]));
				}
			}
		}

		this->test_starting_score_network = new StateNetwork(this->curr_starting_score_network);

		this->test_num_new_inner_states = this->curr_num_new_inner_states+1;
		int curr_total_num_states = this->sum_inner_inputs
			+ this->curr_num_new_inner_states
			+ this->num_sequence_local_states
			+ this->num_sequence_input_states
			+ this->curr_num_new_outer_states;
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
				this->test_state_networks[f_index].push_back(new StateNetwork(
					this->curr_state_networks[f_index][i_index]));
				this->test_state_networks[f_index][i_index]->add_new_inner();
			}
			if (this->is_inner_scope[f_index]) {
				this->test_state_networks[f_index].push_back(new StateNetwork(0,
																			  this->num_sequence_local_states,
																			  this->num_sequence_input_states,
																			  this->sum_inner_inputs+this->test_num_new_inner_states,
																			  this->curr_num_new_outer_states,
																			  20));
			} else {
				this->test_state_networks[f_index].push_back(new StateNetwork(1,
																			  this->num_sequence_local_states,
																			  this->num_sequence_input_states,
																			  this->sum_inner_inputs+this->test_num_new_inner_states,
																			  this->curr_num_new_outer_states,
																			  20));
			}
			for (int s_index = this->sum_inner_inputs+this->curr_num_new_inner_states; s_index < curr_total_num_states; s_index++) {
				this->test_state_networks[f_index].push_back(new StateNetwork(
					this->curr_state_networks[f_index][s_index]));
				this->test_state_networks[f_index][s_index]->add_new_inner();
			}

			this->test_score_networks[f_index] = new StateNetwork(this->curr_score_networks[f_index]);
			this->test_score_networks[f_index]->add_new_inner();
		}

		this->state = FOLD_STATE_ADD_INNER_STATE;
		this->state_iter = 0;
		this->sum_error = 0.0;
	} else {
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.begin();
				it != this->test_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < it->second[n_index].size(); s_index++) {
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
