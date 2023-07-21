#include "branch_experiment.h"

using namespace std;

void BranchExperiment::experiment_transform() {
	double score_standard_deviation = sqrt(*this->existing_score_variance);
	double misguess_standard_deviation = sqrt(*this->existing_misguess_variance);

	cout << "this->existing_average_score: " << *this->existing_average_score << endl;

	double branch_improvement = this->branch_average_score - this->branch_existing_average_score;
	cout << "this->branch_average_score: " << this->branch_average_score << endl;
	cout << "this->branch_existing_average_score: " << this->branch_existing_average_score << endl;

	double replace_improvement = this->replace_average_score - *this->existing_average_score;
	cout << "this->replace_average_score: " << this->replace_average_score << endl;

	double misguess_improvement = *this->existing_average_misguess - this->replace_average_misguess;
	cout << "this->replace_average_misguess: " << this->replace_average_misguess << endl;
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
		// determine if new class needed
		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
				int input_size = this->sequences[a_index]->input_init_types.size();
				this->sequences[a_index]->input_furthest_layer_needed_in = vector<int>(input_size, this->scope_context.size()+2);	// 1 greater than lowest
				this->sequences[a_index]->input_earliest_step_needed_in = vector<int>(input_size, a_index);
				this->sequences[a_index]->input_is_new_class = vector<bool>(input_size, false);
				for (int i_index = 0; i_index < input_size; i_index++) {
					for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
							it != this->sequences[a_index]->state_networks.end(); it++) {
						int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);
						int earliest_step_seen_in = this->scope_earliest_step_seen_in.find(it->first);

						for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
							if (it->second[n_index].size() > 0) {
								StateNetwork* network = it->second[n_index][i_index];
								double sum_impact = 0.0;
								for (int in_index = 0; in_index < 20; in_index++) {
									sum_impact += abs(network->hidden->weights[in_index][0][0]);

									for (int s_index = 0; s_index < (int)network->state_indexes.size(); s_index++) {
										sum_impact += abs(network->hidden->weights[in_index][1][s_index]);
									}
								}

								if (sum_impact > 0.1) {
									// network needed
									if (furthest_layer_seen_in < this->sequences[a_index]->input_furthest_layer_needed_in[i_index]) {
										this->sequences[a_index]->input_furthest_layer_needed_in[i_index] = furthest_layer_seen_in;
									}
									if (earliest_step_seen_in < this->sequences[a_index]->input_earliest_step_needed_in[i_index]) {
										this->sequences[a_index]->input_earliest_step_needed_in[i_index] = earliest_step_seen_in;
									}
								}
							}
						}
					}

					for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
						if (this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
							StateNetwork* network = this->sequences[a_index]->step_state_networks[ia_index][i_index];
							double sum_impact = 0.0;
							for (int in_index = 0; in_index < 20; in_index++) {
								sum_impact += abs(network->hidden->weights[in_index][0][0]);

								for (int s_index = 0; s_index < (int)network->state_indexes.size(); s_index++) {
									sum_impact += abs(network->hidden->weights[in_index][1][s_index]);
								}
							}

							if (sum_impact > 0.1) {
								if (this->scope_context.size()+1 < this->sequences[a_index]->input_furthest_layer_needed_in[i_index]) {
									this->sequences[a_index]->input_furthest_layer_needed_in[i_index] = this->scope_context.size()+1;
								}
								if (ia_index < this->sequences[a_index]->input_earliest_step_needed_in[i_index]) {
									this->sequences[a_index]->input_earliest_step_needed_in[i_index] = ia_index;
								}
							}
						}
					}

					if (this->sequences[a_index]->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
						int previous_used_in = -1;
						for (int ia_index = 0; ia_index < a_index; ia_index++) {
							if (this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
								for (int ii_index = 0; ii_index < (int)this->sequences[ia_index]->input_init_types.size(); ii_index++) {
									if (this->sequences[ia_index]->input_init_types[ii_index] == SEQUENCE_INPUT_INIT_LOCAL) {
										if (this->sequences[ia_index]->input_init_local_scope_depths[ii_index] == this->sequences[a_index]->input_init_local_scope_depths[i_index]
												&& this->sequences[ia_index]->input_init_target_indexes[ii_index] == this->sequences[a_index]->input_init_target_indexes[i_index]) {
											previous_used_in = ia_index;
										}
									}
								}
							}
						}

						if (previous_used_in != -1) {
							if (this->sequences[a_index]->input_furthest_layer_needed_in[i_index] <= this->scope_context.size()+1
									this->sequences[a_index]->input_earliest_step_needed_in[i_index] <= previous_used_in) {
								this->sequences[a_index]->input_is_new_class[i_index] = true;
							}
						} else {
							if (this->sequences[a_index]->input_furthest_layer_needed_in[i_index]
									<= this->scope_context.size() - this->sequences[a_index]->input_init_local_scope_depths[i_index]) {
								this->sequences[a_index]->input_is_new_class[i_index] = true;
							}
						}
					}
				}
			}
		}

		// remove networks if not new class
		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
				int input_size = this->sequences[a_index]->input_init_types.size();
				this->sequences[a_index]->scope_additions_needed = vector<set<int>>(input_size);
				for (int i_index = 0; i_index < input_size; i_index++) {
					if (!this->sequences[a_index]->input_is_new_class[i_index]) {
						// includes SEQUENCE_INPUT_INIT_NONE and SEQUENCE_INPUT_INIT_LAST_SEEN
						for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
								it != this->sequences[a_index]->state_networks.end(); it++) {
							for (int n_index = 0; n_index < it->second.size(); n_index++) {
								if (it->second[n_index].size() > 0) {
									StateNetwork* network = it->second[n_index][i_index];
									double sum_impact = 0.0;
									for (int in_index = 0; in_index < 20; in_index++) {
										sum_impact += abs(network->hidden->weights[in_index][0][0]);

										for (int s_index = 0; s_index < (int)network->state_indexes.size(); s_index++) {
											sum_impact += abs(network->hidden->weights[in_index][1][s_index]);
										}
									}

									if (sum_impact > 0.1) {
										this->sequences[a_index]->scope_additions_needed[i_index].insert(it->first);
									} else {
										delete it->second[n_index][i_index];
										it->second[n_index][i_index] = NULL;
									}
								}
							}
						}

						for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
							if (this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
								StateNetwork* network = this->sequences[a_index]->step_state_networks[ia_index][i_index];
								double sum_impact = 0.0;
								for (int in_index = 0; in_index < 20; in_index++) {
									sum_impact += abs(network->hidden->weights[in_index][0][0]);

									for (int s_index = 0; s_index < (int)network->state_indexes.size(); s_index++) {
										sum_impact += abs(network->hidden->weights[in_index][1][s_index]);
									}
								}

								if (sum_impact < 0.1) {
									delete this->sequences[a_index]->step_state_networks[ia_index][i_index];
									this->sequences[a_index]->step_state_networks[ia_index][i_index] = NULL;
								}
							}
						}
					}
				}
			}
		}

		// remove unnecessary new state networks
		this->scope_additions_needed = vector<set<int>>(NUM_NEW_STATES);
		for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
					it != this->state_networks.end(); it++) {
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						StateNetwork* network = it->second[n_index][s_index];
						double sum_impact = 0.0;
						for (int in_index = 0; in_index < 20; in_index++) {
							sum_impact += abs(network->hidden->weights[in_index][0][0]);

							for (int is_index = 0; is_index < (int)network->state_indexes.size(); is_index++) {
								sum_impact += abs(network->hidden->weights[in_index][1][is_index]);
							}
						}

						if (sum_impact > 0.1) {
							this->scope_additions_needed[s_index].insert(it->first);
						} else {
							delete it->second[n_index][s_index];
							it->second[n_index][s_index] = NULL;
						}
					}
				}
			}

			for (int a_index = 0; a_index < this->num_steps; a_index++) {
				if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
					StateNetwork* network = this->step_state_networks[a_index][s_index];
					double sum_impact = 0.0;
					for (int in_index = 0; in_index < 20; in_index++) {
						sum_impact += abs(network->hidden->weights[in_index][0][0]);

						for (int is_index = 0; is_index < (int)network->state_indexes.size(); is_index++) {
							sum_impact += abs(network->hidden->weights[in_index][1][is_index]);
						}
					}

					if (sum_impact < 0.1) {
						delete this->step_state_networks[a_index][s_index];
						this->step_state_networks[a_index][s_index] = NULL;
					}
				}
			}

			for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
				if (this->exit_network_impacts[e_index] < 0.1) {
					delete this->exit_networks[e_index];
					this->exit_networks[e_index] = NULL;
				}
			}
		}

		// determine new state layers needed in
		this->new_state_furthest_layer_needed_in = vector<int>(NUM_NEW_STATES, this->scope_context.size()+2);
		for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
					it != this->state_networks.end(); it++) {
				int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);

				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						for (int is_index = 0; is_index < NUM_NEW_STATES; is_index++) {
							if (it->second[n_index][is_index] != NULL) {
								if (is_index == s_index) {
									if (furthest_layer_seen_in < this->new_state_furthest_layer_needed_in[s_index]) {
										this->new_state_furthest_layer_needed_in[s_index] = furthest_layer_seen_in;
									}
								} else {
									double sum_state_impact = 0.0;
									for (int in_index = 0; in_index < 20; in_index++) {
										sum_state_impact += abs(it->second[n_index][is_index]->hidden->weights[in_index][2][s_index]);
									}

									if (sum_state_impact > 0.1) {
										if (furthest_layer_seen_in < this->new_state_furthest_layer_needed_in[s_index]) {
											this->new_state_furthest_layer_needed_in[s_index] = furthest_layer_seen_in;
										}
									}
								}
							}
						}
					}
				}
			}

			for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
					it != this->score_networks.end(); it++) {
				int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);

				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index] != NULL) {
						double sum_state_impact = 0.0;
						for (int in_index = 0; in_index < 20; in_index++) {
							sum_state_impact += abs(it->second[n_index]->hidden->weights[in_index][1][s_index]);
						}

						if (sum_state_impact > 0.1) {
							if (furthest_layer_seen_in < this->new_state_furthest_layer_needed_in[s_index]) {
								this->new_state_furthest_layer_needed_in[s_index] = furthest_layer_seen_in;
							}
						}
					}
				}
			}

			for (int a_index = 0; a_index < this->num_steps; a_index++) {
				if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
					for (int is_index = 0; is_index < NUM_NEW_STATES; is_index++) {
						if (is_index == s_index) {
							if (this->step_state_networks[a_index][is_index] != NULL) {
								if (this->scope_context.size()+1 < this->new_state_furthest_layer_seen_in[s_index]) {
									this->new_state_furthest_layer_seen_in[s_index] = this->scope_context.size()+1;
								}
							}
						} else {
							double sum_state_impact = 0.0;
							for (int in_index = 0; in_index < 20; in_index++) {
								sum_state_impact += abs(this->step_state_networks[a_index][is_index]->hidden->weights[in_index][2][s_index]);
							}

							if (sum_state_impact > 0.1) {
								if (this->scope_context.size()+1 < this->new_state_furthest_layer_needed_in[s_index]) {
									this->new_state_furthest_layer_needed_in[s_index] = this->scope_context.size()+1;
								}
							}
						}
					}
				}
			}

			for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
				if (this->exit_networks[e_index] != NULL) {
					double sum_state_impact = 0.0;
					for (int in_index = 0; in_index < 20; in_index++) {
						sum_state_impact += abs(this->exit_networks[e_index]->hidden->weights[in_index][1][s_index]);
					}

					if (sum_state_impact > 0.1) {
						if (this->scope_context.size() < this->new_state_furthest_layer_needed_in[s_index]) {
							this->new_state_furthest_layer_needed_in[s_index] = this->scope_context.size();
						}
					}
				}
			}
		}

		// determine new states needed per layer
		for (int s_index = NUM_NEW_STATES-1; s_index >= 1; s_index--) {
			if (this->new_state_furthest_layer_needed_in[s_index-1] > this->new_state_furthest_layer_needed_in[s_index]) {
				this->new_state_furthest_layer_needed_in[s_index-1] = this->new_state_furthest_layer_needed_in[s_index];
			}
		}
		this->layer_num_new_states = vector<int>(this->scope_context.size()+2);
		for (int l_index = 0; l_index < this->scope_context.size()+2; l_index++) {
			int num_new_states = 0;
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				if (l_index >= this->new_state_furthest_layer_needed_in[s_index]) {
					num_new_states++;
				}
			}
			this->layer_num_new_states[l_index] = num_new_states;
		}

		// clean connections
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
				it != this->state_networks.end(); it++) {
			int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);
			int num_new_states = this->layer_num_new_states[furthest_layer_seen_in];

			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				if (it->second[n_index].size() > 0) {
					for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
						if (it->second[n_index][s_index] != NULL) {
							it->second[n_index][s_index]->clean(num_new_states);
						}
					}
				}
			}
		}

		for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
				it != this->score_networks.end(); it++) {
			int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);
			int num_new_states = this->layer_num_new_states[furthest_layer_seen_in];

			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				if (it->second[n_index].size() > 0) {
					if (it->second[n_index] != NULL) {
						it->second[n_index]->clean(num_new_states);
					}
				}
			}
		}

		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
				for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
					if (this->step_state_networks[a_index][s_index] != NULL) {
						this->step_state_networks[a_index][s_index]->clean(this->layer_num_new_states.back());
					}
				}

				this->step_score_networks[a_index]->clean(this->layer_num_new_states.back());
			}
		}

		for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
			if (this->exit_networks[e_index] != NULL) {
				this->exit_networks[e_index]->clean(this->layer_num_new_states[this->scope_context.size()]);
			}
		}

		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
				int input_size = this->sequences[a_index]->input_init_types.size();
				for (int i_index = 0; i_index < input_size; i_index++) {
					if (!this->sequences[a_index]->input_is_new_class[i_index]) {
						// includes SEQUENCE_INPUT_INIT_NONE and SEQUENCE_INPUT_INIT_LAST_SEEN
						for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
								it != this->sequences[a_index]->state_networks.end(); it++) {
							int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);
							int num_new_states = this->layer_num_new_states[furthest_layer_seen_in];

							for (int n_index = 0; n_index < it->second.size(); n_index++) {
								if (it->second[n_index].size() > 0) {
									if (it->second[n_index][i_index] != NULL) {
										it->second[n_index][i_index]->clean(num_new_states);
									}
								}
							}
						}

						for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
							if (this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
								if (this->sequences[a_index]->step_state_networks[ia_index][i_index] != NULL) {
									this->sequences[a_index]->step_state_networks[ia_index][i_index]->clean(this->layer_num_new_states.back());
								}
							}
						}
					}
				}
			}
		}

		this->state = BRANCH_EXPERIMENT_STATE_FIRST_CLEAN;
		this->state_iter = 0;
		this->sum_error = 0.0;
	} else {

	}
}
