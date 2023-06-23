#include "branch_experiment.h"

using namespace std;

void BranchExperiment::experiment_transform() {
	double score_standard_deviation = sqrt(*this->existing_score_variance);
	double misguess_standard_deviation = sqrt(*this->existing_misguess_variance);

	cout << "this->existing_average_score: " << *this->existing_average_score << endl;

	double branch_improvement = this->test_branch_average_score - this->test_branch_existing_average_score;
	cout << "this->test_branch_average_score: " << this->test_branch_average_score << endl;
	cout << "this->test_branch_existing_average_score: " << this->test_branch_existing_average_score << endl;

	double replace_improvement = this->test_replace_average_score - *this->existing_average_score;
	cout << "this->test_replace_average_score: " << this->test_replace_average_score << endl;

	double misguess_improvement = *this->existing_average_misguess - this->test_replace_average_misguess;
	cout << "this->test_replace_average_misguess: " << this->test_replace_average_misguess << endl;
	cout << "this->existing_average_misguess: " << *this->existing_average_misguess << endl;

	// 0.0001 rolling average variance approx. equal to 20000 average variance (?)

	double branch_improvement_t_value = branch_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "branch_improvement_t_value: " << branch_improvement_t_value << endl;

	double replace_improvement_t_value = replace_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "replace_improvement_t_value: " << replace_improvement_t_value << endl;

	double misguess_improvement_t_value = misguess_improvement
		/ (misguess_standard_deviation / sqrt(20000));
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	if (branch_improvement_t_value > 2.326) {	// >99%
		if (replace_improvement_t_value > -0.842	// 80%<
				&& this->is_recursive == 0) {
			cout << "FOLD_RESULT_REPLACE" << endl;
			this->experiment_result = BRANCH_EXPERIMENT_RESULT_REPLACE;
		} else {
			cout << "FOLD_RESULT_BRANCH" << endl;
			this->experiment_result = BRANCH_EXPERIMENT_RESULT_BRANCH;
		}
	} else if (*this->existing_average_misguess > 0.01
			&& misguess_improvement_t_value > 2.326	// >99%
			&& replace_improvement_t_value > -0.842	// 80%<
			&& this->is_recursive == 0) {
		cout << "FOLD_RESULT_REPLACE" << endl;
		this->experiment_result = BRANCH_EXPERIMENT_RESULT_REPLACE;
	} else {
		cout << "FOLD_RESULT_FAIL" << endl;
		this->experiment_result = BRANCH_EXPERIMENT_RESULT_FAIL;
	}

	if (this->experiment_result != BRANCH_EXPERIMENT_RESULT_FAIL) {
		vector<double> sum_state_impacts(this->num_new_states, 0.0);
		vector<bool> new_states_needed(this->num_new_states);

		for (int s_index = 0; s_index < this->num_new_states; s_index++) {
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
					it != this->state_networks.end(); it++) {
				for (int n_index = 0; n_index < it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						for (int is_index = 0; is_index < this->num_new_states; is_index++) {
							for (int in_index = 0; in_index < 20; in_index++) {
								sum_state_impacts[s_index] += abs(it->second[n_index][is_index]->hidden->weights[in_index][2][s_index]);
							}
						}
					}
				}
			}

			for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
					it != this->score_networks.end(); it++) {
				for (int n_index = 0; n_index < it->second.size(); n_index++) {
					if (it->second[n_index] != NULL) {
						for (int in_index = 0; in_index < 20; in_index++) {
							sum_state_impacts[s_index] += abs(it->second[n_index]->hidden->weights[in_index][1][s_index]);
						}
					}
				}
			}

			for (int in_index = 0; in_index < 20; in_index++) {
				sum_state_impacts[s_index] += abs(this->starting_score_network->hidden->weights[in_index][1][s_index]);
			}

			for (int a_index = 0; a_index < this->num_steps; a_index++) {
				if (this->step_types[a_index] == EXPLORE_STEP_TYPE_ACTION) {
					for (int is_index = 0; is_index < this->num_new_states; is_index++) {
						for (int in_index = 0; in_index < 20; in_index++) {
							sum_state_impacts[s_index] += abs(this->step_state_networks[a_index][is_index]->hidden->weights[in_index][2][s_index]);
						}
					}

					for (int in_index = 0; in_index < 20; in_index++) {
						sum_state_impacts[s_index] += abs(this->step_score_networks[a_index]->hidden->weights[in_index][1][s_index]);
					}
				}
			}

			for (int es_index = 0; es_index < (int)this->exit_networks.size(); es_index++) {
				for (map<StateDefinition*, ExitNetwork*>::iterator it = this->exit_networks[es_index].begin();
						it != this->exit_networks[es_index].end(); it++) {
					for (int in_index = 0; in_index < 20; in_index++) {
						sum_state_impacts[s_index] += abs(it->second->hidden->weights[in_index][1][s_index]);
					}
				}
			}

			if (sum_state_impacts[s_index] < 0.5) {
				new_states_needed[s_index] = false;
			} else {
				new_states_needed[s_index] = true;
			}
		}

		int cleaned_num_states = this->num_new_states;
		for (int s_index = this->num_new_states-1; s_index >= 0; s_index--) {
			if (new_states_needed[s_index]) {
				break;
			} else {
				cleaned_num_states--;
			}
		}

		this->new_state_furthest_layer_seen_in = vector<int>(cleaned_num_states, (int)this->scope_context.size()+1);

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
				it != this->state_networks.end(); it++) {
			for (int n_index = 0; n_index < it->second.size(); n_index++) {
				if (it->second[n_index].size() > 0) {
					for (int s_index = 0; s_index < this->num_new_states; s_index++) {
						bool network_needed = true;
						if (s_index >= cleaned_num_states) {
							network_needed = false;
						} else {
							double sum_obs_impact = 0.0;
							for (int in_index = 0; in_index < 20; in_index++) {
								sum_obs_impact += abs(it->second[n_index][s_index]->hidden->weights[in_index][0][0]);
							}

							if (sum_obs_impact < 0.1) {
								network_needed = false;
							}
						}

						if (!network_needed) {
							delete it->second[n_index][s_index];
							it->second[n_index][s_index] = NULL;
						} else {
							int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first)->second;
							if (furthest_layer_seen_in < this->new_state_furthest_layer_seen_in[s_index]) {
								this->new_state_furthest_layer_seen_in[s_index] = furthest_layer_seen_in;
							}
						}
					}
				}
			}
		}

		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == EXPLORE_STEP_TYPE_ACTION) {
				for (int s_index = 0; s_index < this->num_new_states; s_index++) {
					bool network_needed = true;
					if (s_index >= cleaned_num_states) {
						network_needed = false;
					} else {
						double sum_obs_impact = 0.0;
						for (int in_index = 0; in_index < 20; in_index++) {
							sum_obs_impact += abs(this->step_state_networks[a_index][s_index]->hidden->weights[in_index][0][0]);
						}

						if (sum_obs_impact < 0.1) {
							network_needed = false;
						}
					}

					if (!network_needed) {
						delete this->step_state_networks[a_index][s_index];
						this->step_state_networks[a_index][s_index] = NULL;
					}
				}
			}
		}

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
				it != this->state_networks.end(); it++) {
			for (int n_index = 0; n_index < it->second.size(); n_index++) {
				if (it->second[n_index].size() > 0) {
					for (int s_index = 0; s_index < cleaned_num_states; s_index++) {
						if (it->second[n_index][s_index] != NULL) {
							it->second[n_index][s_index]->clean(cleaned_num_states);
						}
					}
				}
			}
		}

		for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
				it != this->score_networks.end(); it++) {
			for (int n_index = 0; n_index < it->second.size(); n_index++) {
				if (it->second[n_index] != NULL) {
					it->second[n_index]->clean(cleaned_num_states);
				}
			}
		}

		this->starting_score_network->clean(cleaned_num_states);

		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == EXPLORE_STEP_TYPE_ACTION) {
				for (int s_index = 0; s_index < cleaned_num_states; s_index++) {
					if (this->step_state_networks[a_index][s_index] != NULL) {
						this->step_state_networks[a_index][s_index]->clean(cleaned_num_states);
					}
				}


			}
		}

		for (int es_index = 0; es_index < (int)this->exit_networks.size(); es_index++) {
			for (map<StateDefinition*, ExitNetwork*>::iterator it = this->exit_networks[es_index].begin();
					it != this->exit_networks[es_index].end(); it++) {
				it->second->clean(cleaned_num_states);
			}
		}



		this->num_new_states = cleaned_num_states;
	} else {

	}
}
