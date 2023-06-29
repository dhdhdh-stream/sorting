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
		// determine if new types needed
		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
				int input_size = this->sequences[a_index]->input_init_types.size();
				this->sequences[a_index]->input_furthest_layer_seen_in = vector<int>(input_size, this->scope_context.size()+2);	// 1 greater than lowest
				this->sequences[a_index]->input_steps_seen_in = vector<vector<bool>>(input_size, vector<bool>(this->num_steps, false));
				this->sequences[a_index]->input_is_new_class = vector<bool>(input_size, false);
				for (int i_index = 0; i_index < input_size; i_index++) {
					for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
							it != this->sequences[a_index]->state_networks.end(); it++) {
						int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);
						vector<bool> steps_seen_in = this->scope_steps_seen_in.find(it->first);

						for (int n_index = 0; n_index < it->second.size(); n_index++) {
							if (it->second[n_index].size() > 0) {
								double sum_obs_impact = 0.0;
								for (int in_index = 0; in_index < 20; in_index++) {
									sum_obs_impact += abs(it->second[n_index][i_index]->hidden->weights[in_index][0][0]);
								}

								if (sum_obs_impact > 0.1) {
									// network needed
									if (furthest_layer_seen_in < this->sequences[a_index]->input_furthest_layer_seen_in[i_index]) {
										this->sequences[a_index]->input_furthest_layer_seen_in[i_index] = furthest_layer_seen_in;
									}
									for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
										if (steps_seen_in[ia_index]) {
											this->sequences[a_index]->input_steps_seen_in[i_index][ia_index] = true;
										}
									}
								}
							}
						}
					}

					for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
						if (this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
							double sum_obs_impact = 0.0;
							for (int in_index = 0; in_index < 20; in_index++) {
								sum_obs_impact += abs(this->sequences[a_index]->step_state_networks[ia_index][i_index]->hidden->weights[in_index][0][0]);
							}

							if (sum_obs_impact > 0.1) {
								this->sequences[a_index]->input_steps_seen_in[i_index][ia_index] = true;
							}
						} else {
							// this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE
							for (int s_index = 0; s_index < (int)this->sequences[ia_index]->scopes.size(); s_index++) {
								Scope* scope = solution->scopes[this->sequences[ia_index]->scopes[s_index]];
								for (int n_index = 0; n_index < (int)this->sequences[ia_index]->node_ids[s_index].size(); n_index++) {
									if (scope->nodes[this->sequences[ia_index]->node_ids[s_index][n_index]]->type == NODE_TYPE_ACTION) {
										double sum_obs_impact = 0.0;
										for (int in_index = 0; in_index < 20; in_index++) {
											sum_obs_impact += abs(this->sequences[a_index]->sequence_state_networks
												[ia_index][s_index][n_index][i_index]->hidden->weights[in_index][0][0]);
										}

										if (sum_obs_impact > 0.1) {
											this->sequences[a_index]->input_steps_seen_in[i_index][ia_index] = true;
										}
									}
								}
							}
						}
					}

					if (this->sequences[a_index]->input_init_types[i_index] == SEQUENCE_INPUT_INIT_NONE) {
						if (this->sequences[a_index]->input_furthest_layer_seen_in[i_index] < this->scope_context.size()+2) {
							this->sequences[a_index]->input_is_new_class[i_index] = true;
						}
					} else if (this->sequences[a_index]->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
						if (this->sequences[a_index]->input_furthest_layer_seen_in[i_index]
								<= this->scope_context.size() - this->sequences[a_index]->input_init_local_scope_depths[i_index]) {
							this->sequences[a_index]->input_is_new_class[i_index] = true;
						}
					} else if (this->sequences[a_index]->input_init_types[i_index] == SEQUENCE_INPUT_INIT_PREVIOUS) {
						bool is_new_class = false;
						if (this->sequences[a_index]->input_furthest_layer_seen_in[i_index] < this->scope_context.size()+2) {
							is_new_class = true;
						} else {
							for (int ia_index = 0; ia_index <= this->sequences[a_index]->input_init_previous_step_indexes[i_index]; ia_index++) {
								if (this->sequences[a_index]->input_steps_seen_in[i_index][ia_index]) {
									is_new_class = true;
									break;
								}
							}
						}
						this->sequences[a_index]->input_is_new_class[i_index] = is_new_class;
					} else {
						// this->sequences[a_index]->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LAST_SEEN
						if (this->sequences[a_index]->input_furthest_layer_seen_in[i_index] < this->scope_context.size()+2) {
							this->sequences[a_index]->input_is_new_class[i_index] = true;
						}
					}
				}
			}
		}

		// remove networks if not new type
		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
				for (int i_index = 0; i_index < input_size; i_index++) {
					if (!this->sequences[a_index]->input_is_new_class[i_index]) {
						for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
								it != this->sequences[a_index]->state_networks.end(); it++) {
							for (int n_index = 0; n_index < it->second.size(); n_index++) {
								if (it->second[n_index].size() > 0) {
									double sum_obs_impact = 0.0;
									for (int in_index = 0; in_index < 20; in_index++) {
										sum_obs_impact += abs(it->second[n_index][i_index]->hidden->weights[in_index][0][0]);
									}

									if (sum_obs_impact < 0.1) {
										delete it->second[n_index][i_index];
										it->second[n_index][i_index] = NULL;
									}
								}
							}
						}

						for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
							double sum_obs_impact = 0.0;
							for (int in_index = 0; in_index < 20; in_index++) {
								sum_obs_impact += abs(this->sequences[a_index]->step_state_networks[ia_index][i_index]->hidden->weights[in_index][0][0]);
							}

							if (sum_obs_impact < 0.1) {
								delete this->sequences[a_index]->step_state_networks[ia_index][i_index];
								this->sequences[a_index]->step_state_networks[ia_index][i_index] = NULL;
							}
						}

						for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
							if (this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
								for (int s_index = 0; s_index < (int)this->sequences[ia_index]->scopes.size(); s_index++) {
									Scope* scope = solution->scopes[this->sequences[ia_index]->scopes[s_index]];
									for (int n_index = 0; n_index < (int)this->sequences[ia_index]->node_ids[s_index].size(); n_index++) {
										if (scope->nodes[this->sequences[ia_index]->node_ids[s_index][n_index]]->type == NODE_TYPE_ACTION) {
											double sum_obs_impact = 0.0;
											for (int in_index = 0; in_index < 20; in_index++) {
												sum_obs_impact += abs(this->sequences[a_index]->sequence_state_networks
													[ia_index][s_index][n_index][i_index]->hidden->weights[in_index][0][0]);
											}

											if (sum_obs_impact < 0.1) {
												delete this->sequences[a_index]->sequence_state_networks[ia_index][s_index][n_index][i_index];
												this->sequences[a_index]->sequence_state_networks[ia_index][s_index][n_index][i_index] = NULL;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// remove unnecessary new state
		this->new_state_furthest_layer_seen_in = vector<int>(NUM_NEW_STATES, this->scope_context.size()+2);
		for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
					it != this->state_networks.end(); it++) {
				int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);

				for (int n_index = 0; n_index < it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						for (int is_index = 0; is_index < NUM_NEW_STATES; is_index++) {
							double sum_obs_impact = 0.0;
							for (int in_index = 0; in_index < 20; in_index++) {
								sum_obs_impact += abs(it->second[n_index][is_index]->hidden->weights[in_index][0][0]);
							}

							if (sum_obs_impact > 0.1) {
								if (furthest_layer_seen_in < this->new_state_furthest_layer_seen_in[s_index]) {
									this->new_state_furthest_layer_seen_in[s_index] = furthest_layer_seen_in;
								}
							} else {
								delete it->second[n_index][is_index];
								it->second[n_index][is_index] = NULL;
							}
						}
					}
				}
			}

			for (int a_index = 0; a_index < this->num_steps; a_index++) {
				if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
					double sum_obs_impact = 0.0;
					for (int in_index = 0; in_index < 20; in_index++) {
						sum_obs_impact += abs(this->step_state_networks[a_index][s_index]->hidden->weights[in_index][0][0]);
					}

					if (sum_obs_impact > 0.1) {
						if (this->scopes.size()+1 < this->new_state_furthest_layer_seen_in[s_index]) {
							this->new_state_furthest_layer_seen_in[s_index] = this->scopes.size()+1;
						}
					} else {
						delete this->step_state_networks[a_index][s_index];
						this->step_state_networks[a_index][s_index] = NULL;
					}
				} else {
					// this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE
					for (int is_index = 0; is_index < (int)this->sequences[a_index]->scopes.size(); is_index++) {
						Scope* scope = solution->scopes[this->sequences[a_index]->scopes[is_index]];
						for (int n_index = 0; n_index < (int)this->sequences[a_index]->node_ids[is_index].size(); n_index++) {
							if (scope->nodes[this->sequences[a_index]->node_ids[is_index][n_index]]->type == NODE_TYPE_ACTION) {
								double sum_obs_impact = 0.0;
								for (int in_index = 0; in_index < 20; in_index++) {
									sum_obs_impact += abs(this->sequence_state_networks[a_index][is_index]
										[this->sequences[a_index]->node_ids[is_index][n_index]][s_index]->hidden->weights[in_index][0][0]);
								}

								if (sum_obs_impact > 0.1) {
									if (this->scopes.size()+1 < this->new_state_furthest_layer_seen_in[s_index]) {
										this->new_state_furthest_layer_seen_in[s_index] = this->scopes.size()+1;
									}
								} else {
									delete this->sequence_state_networks[a_index][is_index][this->sequences[a_index]->node_ids[is_index][n_index]][s_index];
									this->sequence_state_networks[a_index][is_index][this->sequences[a_index]->node_ids[is_index][n_index]][s_index] = NULL;
								}
							}
						}
					}
				}
			}
		}

		for (int s_index = NUM_NEW_STATES-1; s_index >= 1; s_index--) {
			if (this->new_state_furthest_layer_seen_in[s_index-1] > this->new_state_furthest_layer_seen_in[s_index]) {
				this->new_state_furthest_layer_seen_in[s_index-1] = this->new_state_furthest_layer_seen_in[s_index];
			}
		}
		this->layer_num_new_states = vector<int>(this->scope_context.size()+2);
		for (int l_index = 0; l_index < this->scope_context.size()+2; l_index++) {
			int num_new_states = 0;
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				if (l_index >= this->new_state_furthest_layer_seen_in[s_index]) {
					num_new_states++;
				}
			}
			this->layer_num_new_states[l_index] = num_new_states;
		}

		// clean connections
		

		this->state = EXPERIMENT_STATE_NEW_CLASSES;
		this->state_iter = 0;
		this->sum_error = 0.0;
	} else {

	}
}
