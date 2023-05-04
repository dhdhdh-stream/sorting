#include "fold.h"

#include <cmath>
#include <iostream>

#include "globals.h"

using namespace std;

void Fold::remove_inner_input_end() {
	// TODO: consider only branching when correct for remove_inner_input
	double score_improvement = this->test_branch_average_score - this->curr_branch_average_score;
	cout << "score_improvement: " << score_improvement << endl;

	double misguess_improvement = this->curr_replace_average_misguess - this->test_replace_average_misguess;
	cout << "misguess_improvement: " << misguess_improvement << endl;

	// TODO: check if need to add and use curr standard deviations
	double score_standard_deviation = sqrt(*this->existing_score_variance);
	double misguess_standard_deviation = sqrt(*this->existing_misguess_variance);

	double score_improvement_t_value = score_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "score_improvement_t_value: " << score_improvement_t_value << endl;

	double misguess_improvement_t_value = misguess_improvement
		/ (misguess_standard_deviation / sqrt(20000));
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	// if (score_improvement_t_value > -0.842 && misguess_improvement_t_value > -0.842) {
	if (rand()%2 == 0) {
		cout << "REMOVE_INNER_INPUT success" << endl;

		// no change to outer

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->curr_state_networks[f_index].size(); s_index++) {
				if (this->curr_state_networks[f_index][s_index] != NULL) {
					delete this->curr_state_networks[f_index][s_index];
				}
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

		this->curr_inner_inputs_needed = this->test_inner_inputs_needed;
		this->curr_state_networks_not_needed = this->test_state_networks_not_needed;
	} else {
		cout << "REMOVE_INNER_INPUT fail" << endl;

		// no change to outer

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			for (int s_index = 0; s_index < (int)this->test_state_networks[f_index].size(); s_index++) {
				if (this->test_state_networks[f_index][s_index] != NULL) {
					delete this->test_state_networks[f_index][s_index];
				}
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

		this->test_inner_inputs_needed = this->curr_inner_inputs_needed;
		this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
	}

	this->remove_inner_input_index++;

	if (this->remove_inner_input_index >= this->sum_inner_inputs) {
		cout << "ending REMOVE_INNER_INPUT" << endl;
		cout << "EXPERIMENT_DONE" << endl;

		this->state = FOLD_STATE_EXPERIMENT_DONE;
	} else {
		this->test_inner_inputs_needed[this->remove_inner_input_index] = false;
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			this->test_state_networks_not_needed[f_index][this->remove_inner_input_index] = true;
		}

		// outer unchanged

		int num_inner_networks = this->sum_inner_inputs
			+ this->curr_num_new_inner_states
			+ this->num_sequence_states;
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

		cout << "ending REMOVE_INNER_INPUT" << endl;
		cout << "starting REMOVE_INNER_INPUT " << this->remove_inner_input_index << endl;

		this->state = FOLD_STATE_REMOVE_INNER_INPUT;
		this->state_iter = 0;
		this->sum_error = 0.0;
	}
}
