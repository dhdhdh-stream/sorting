#include <iostream>

#include "globals.h"

using namespace std;

void LoopFold::remove_inner_input_end() {
	double score_improvement = this->test_average_score - this->curr_average_score;
	cout << "score_improvement: " << score_improvement << endl;

	double misguess_improvement = this->curr_average_misguess - this->test_average_misguess;
	cout << "misguess_improvement: " << misguess_improvement << endl;

	// TODO: check if should instead use existing
	double score_standard_deviation = sqrt(this->curr_score_variance);
	double misguess_standard_deviation = sqrt(this->curr_misguess_variance);

	double score_improvement_t_value = score_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "score_improvement_t_value: " << score_improvement_t_value << endl;

	double misguess_improvement_t_value = misguess_improvement
		/ (misguess_standard_deviation / sqrt(20000));
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	if (score_improvement_t_value > -0.842 && misguess_improvement_t_value > -0.842) {
		cout << "REMOVE_INNER_INPUT success" << endl;

		// no change to outer

		for (int i_index = 0; i_index < (int)this->curr_starting_state_networks.size(); i_index++) {
			if (this->curr_starting_state_networks[i_index] != NULL) {
				delete this->curr_starting_state_networks[i_index];
			}
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

		this->curr_average_score = this->test_average_score;
		this->test_average_score = 0.0;
		this->curr_score_variance = this->test_score_variance;
		this->test_score_variance = 0.0;
		this->curr_average_misguess = this->test_average_misguess;
		this->test_average_misguess = 0.0;
		this->curr_misguess_variance = this->test_misguess_variance;
		this->test_misguess_variance = 0.0;

		this->curr_inner_inputs_needed = this->test_inner_inputs_needed;
		this->curr_state_networks_not_needed = this->test_state_networks_not_needed;
	} else {
		cout << "REMOVE_INNER_INPUT fail" << endl;

		// no change to outer

		for (int i_index = 0; i_index < (int)this->test_starting_state_networks.size(); i_index++) {
			if (this->test_starting_state_networks[i_index] != NULL) {
				delete this->test_starting_state_networks[i_index];
			}
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

		for (int i_index = 0; i_index < this->sum_inner_inputs+this->curr_num_new_inner_states; i_index++) {
			if (i_index >= this->sum_inner_inputs
					|| this->test_inner_inputs_needed[i_index]) {
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

		int num_inner_networks = this->sum_inner_inputs
			+ this->curr_num_new_inner_states
			+ this->num_states;
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

		this->state = LOOP_FOLD_STATE_REMOVE_INNER_INPUT;
		this->sub_state = LOOP_FOLD_SUB_STATE_LEARN;
		this->state_iter = 0;
		this->sum_error = 0.0;
	}
}
