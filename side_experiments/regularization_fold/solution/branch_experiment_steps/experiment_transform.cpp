#include "branch_experiment.h"

#include <cmath>
#include <iostream>

#include "exit_network.h"
#include "globals.h"
#include "layer.h"
#include "scale.h"
#include "scope.h"
#include "sequence.h"
#include "score_network.h"
#include "state_network.h"

using namespace std;

void BranchExperiment::experiment_transform() {
	cout << "experiment_transform" << endl;

	double score_improvement = this->new_average_score - this->existing_average_score;
	cout << "this->new_average_score: " << this->new_average_score << endl;
	cout << "this->existing_average_score: " << this->existing_average_score << endl;

	double misguess_improvement = this->existing_average_misguess - this->new_average_misguess;
	cout << "this->new_average_misguess: " << this->new_average_misguess << endl;
	cout << "this->existing_average_misguess: " << this->existing_average_misguess << endl;

	// 0.0001 rolling average variance approx. equal to 20000 average variance (?)

	double score_standard_deviation = sqrt(solution->score_variance);
	double score_improvement_t_value = score_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "score_improvement_t_value: " << score_improvement_t_value << endl;

	double misguess_improvement_t_value = misguess_improvement
		/ (solution->misguess_standard_deviation / sqrt(20000));
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	bool is_success;
	if (score_improvement_t_value > 2.326	// >99%
			&& this->branch_weight > 0.05) {
		is_success = true;
	} else if (score_improvement_t_value > -0.674	// 75%<
			&& misguess_improvement_t_value > 2.326
			&& this->branch_weight > 0.05) {
		is_success = true;
	} else {
		is_success = false;
	}

	// temp
	is_success = true;

	if (is_success) {
		// determine if new class needed
		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
				int input_size = (int)this->sequences[a_index]->input_types.size();
				this->sequences[a_index]->input_furthest_layer_needed_in = vector<int>(input_size, (int)this->scope_context.size()+2);	// 1 greater than lowest
				this->sequences[a_index]->input_steps_needed_in = vector<vector<bool>>(input_size, vector<bool>(this->num_steps, false));
				this->sequences[a_index]->input_is_new_class = vector<bool>(input_size, false);
				for (int i_index = 0; i_index < input_size; i_index++) {
					int earliest_step_needed_in = a_index;

					for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
							it != this->sequences[a_index]->state_networks.end(); it++) {
						int furthest_layer_seen_in = this->scope_furthest_layer_seen_in[it->first];
						vector<bool> steps_seen_in = this->scope_steps_seen_in[it->first];

						for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
							if (it->second[n_index].size() > 0) {
								StateNetwork* network = it->second[n_index][i_index];
								double sum_impact = 0.0;
								for (int in_index = 0; in_index < 20; in_index++) {
									if (abs(network->output->weights[0][0][in_index]) > 0.05) {
										sum_impact += abs(network->hidden->weights[in_index][0][0]);

										for (int s_index = 0; s_index < (int)network->state_indexes.size(); s_index++) {
											sum_impact += abs(network->hidden->weights[in_index][1][s_index]);
										}
									}
								}

								if (sum_impact > 0.1) {
									// network needed
									if (furthest_layer_seen_in < this->sequences[a_index]->input_furthest_layer_needed_in[i_index]) {
										this->sequences[a_index]->input_furthest_layer_needed_in[i_index] = furthest_layer_seen_in;
									}

									for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
										if (steps_seen_in[ia_index]) {
											this->sequences[a_index]->input_steps_needed_in[i_index][ia_index] = true;

											if (ia_index < earliest_step_needed_in) {
												earliest_step_needed_in = ia_index;
											}
										}
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
								if (abs(network->output->weights[0][0][in_index]) > 0.05) {
									sum_impact += abs(network->hidden->weights[in_index][0][0]);

									for (int s_index = 0; s_index < (int)network->state_indexes.size(); s_index++) {
										sum_impact += abs(network->hidden->weights[in_index][1][s_index]);
									}
								}
							}

							if (sum_impact > 0.1) {
								if ((int)this->scope_context.size()+1 < this->sequences[a_index]->input_furthest_layer_needed_in[i_index]) {
									this->sequences[a_index]->input_furthest_layer_needed_in[i_index] = (int)this->scope_context.size()+1;
								}

								if (ia_index < earliest_step_needed_in) {
									earliest_step_needed_in = ia_index;
								}
							}
						}
					}

					if (this->sequences[a_index]->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
						if (this->sequences[a_index]->input_furthest_layer_needed_in[i_index]
								<= (int)this->scope_context.size() - this->sequences[a_index]->input_local_scope_depths[i_index]) {
							this->sequences[a_index]->input_is_new_class[i_index] = true;
						}
					} else if (this->sequences[a_index]->input_types[i_index] == SEQUENCE_INPUT_TYPE_PREVIOUS) {
						if (this->sequences[a_index]->input_furthest_layer_needed_in[i_index] < (int)this->scope_context.size()+1
								|| earliest_step_needed_in <= this->sequences[a_index]->input_previous_step_index[i_index]) {
							this->sequences[a_index]->input_is_new_class[i_index] = true;
						}
					}
				}
			}
		}

		// remove networks if not new class
		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
				int input_size = (int)this->sequences[a_index]->input_types.size();
				this->sequences[a_index]->scope_additions_needed = vector<set<int>>(input_size);
				for (int i_index = 0; i_index < input_size; i_index++) {
					if (!this->sequences[a_index]->input_is_new_class[i_index]) {
						// includes SEQUENCE_INPUT_TYPE_NONE and SEQUENCE_INPUT_TYPE_LAST_SEEN
						for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
								it != this->sequences[a_index]->state_networks.end(); it++) {
							for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
								if (it->second[n_index].size() > 0) {
									StateNetwork* network = it->second[n_index][i_index];
									double sum_impact = 0.0;
									for (int in_index = 0; in_index < 20; in_index++) {
										if (abs(network->output->weights[0][0][in_index]) > 0.05) {
											sum_impact += abs(network->hidden->weights[in_index][0][0]);

											for (int s_index = 0; s_index < (int)network->state_indexes.size(); s_index++) {
												sum_impact += abs(network->hidden->weights[in_index][1][s_index]);
											}
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
									if (abs(network->output->weights[0][0][in_index]) > 0.05) {
										sum_impact += abs(network->hidden->weights[in_index][0][0]);

										for (int s_index = 0; s_index < (int)network->state_indexes.size(); s_index++) {
											sum_impact += abs(network->hidden->weights[in_index][1][s_index]);
										}
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
		for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
					it != this->state_networks.end(); it++) {
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						StateNetwork* network = it->second[n_index][s_index];
						double sum_impact = 0.0;
						for (int in_index = 0; in_index < 20; in_index++) {
							if (abs(network->output->weights[0][0][in_index]) > 0.05) {
								sum_impact += abs(network->hidden->weights[in_index][0][0]);

								for (int is_index = 0; is_index < (int)network->state_indexes.size(); is_index++) {
									sum_impact += abs(network->hidden->weights[in_index][1][is_index]);
								}
							}
						}

						if (sum_impact < 0.1) {
							delete it->second[n_index][s_index];
							it->second[n_index][s_index] = NULL;
						} else {
							cout << "keep " << it->first << " " << n_index << " " << s_index << ": " << sum_impact << endl;
						}
					}
				}
			}

			for (int a_index = 0; a_index < this->num_steps; a_index++) {
				if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
					StateNetwork* network = this->step_state_networks[a_index][s_index];
					double sum_impact = 0.0;
					for (int in_index = 0; in_index < 20; in_index++) {
						if (abs(network->output->weights[0][0][in_index]) > 0.05) {
							sum_impact += abs(network->hidden->weights[in_index][0][0]);

							for (int is_index = 0; is_index < (int)network->state_indexes.size(); is_index++) {
								sum_impact += abs(network->hidden->weights[in_index][1][is_index]);
							}
						}
					}

					if (sum_impact < 0.1) {
						delete this->step_state_networks[a_index][s_index];
						this->step_state_networks[a_index][s_index] = NULL;
					} else {
						cout << "keep " << a_index << " " << s_index << ": " << sum_impact << endl;
					}
				}
			}
		}

		for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
			if (this->exit_network_impacts[e_index] < 0.1) {
				delete this->exit_networks[e_index];
				this->exit_networks[e_index] = NULL;
			}
		}

		// determine new state layers needed in
		this->new_state_furthest_layer_needed_in = vector<int>(NUM_NEW_STATES, (int)this->scope_context.size()+2);
		this->new_state_steps_needed_in = vector<vector<bool>>(NUM_NEW_STATES, vector<bool>(this->num_steps, false));
		this->scope_additions_needed = vector<set<int>>(NUM_NEW_STATES);
		for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
					it != this->state_networks.end(); it++) {
				int furthest_layer_seen_in = this->scope_furthest_layer_seen_in[it->first];
				vector<bool> steps_seen_in = this->scope_steps_seen_in[it->first];

				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						for (int is_index = 0; is_index < NUM_NEW_STATES; is_index++) {
							if (it->second[n_index][is_index] != NULL) {
								if (is_index == s_index) {
									this->scope_additions_needed[s_index].insert(it->first);

									if (furthest_layer_seen_in < this->new_state_furthest_layer_needed_in[s_index]) {
										this->new_state_furthest_layer_needed_in[s_index] = furthest_layer_seen_in;
									}

									for (int a_index = 0; a_index < this->num_steps; a_index++) {
										if (steps_seen_in[a_index]) {
											this->new_state_steps_needed_in[s_index][a_index] = true;
										}
									}
								} else {
									double sum_state_impact = 0.0;
									for (int in_index = 0; in_index < 20; in_index++) {
										if (abs(it->second[n_index][is_index]->output->weights[0][0][in_index]) > 0.05) {
											sum_state_impact += abs(it->second[n_index][is_index]->hidden->weights[in_index][2][s_index]);
										}
									}

									if (sum_state_impact > 0.1) {
										this->scope_additions_needed[s_index].insert(it->first);

										if (furthest_layer_seen_in < this->new_state_furthest_layer_needed_in[s_index]) {
											this->new_state_furthest_layer_needed_in[s_index] = furthest_layer_seen_in;
										}

										for (int a_index = 0; a_index < this->num_steps; a_index++) {
											if (steps_seen_in[a_index]) {
												this->new_state_steps_needed_in[s_index][a_index] = true;
											}
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
				int furthest_layer_seen_in = this->scope_furthest_layer_seen_in[it->first];
				vector<bool> steps_seen_in = this->scope_steps_seen_in[it->first];

				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index] != NULL) {
						double sum_state_impact = 0.0;
						for (int in_index = 0; in_index < 20; in_index++) {
							sum_state_impact += abs(it->second[n_index]->hidden->weights[in_index][1][s_index]);
						}

						if (sum_state_impact > 0.1) {
							this->scope_additions_needed[s_index].insert(it->first);

							if (furthest_layer_seen_in < this->new_state_furthest_layer_needed_in[s_index]) {
								this->new_state_furthest_layer_needed_in[s_index] = furthest_layer_seen_in;
							}

							for (int a_index = 0; a_index < this->num_steps; a_index++) {
								if (steps_seen_in[a_index]) {
									this->new_state_steps_needed_in[s_index][a_index] = true;
								}
							}
						}
					}
				}
			}

			for (int a_index = 0; a_index < this->num_steps; a_index++) {
				if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
					for (int is_index = 0; is_index < NUM_NEW_STATES; is_index++) {
						if (this->step_state_networks[a_index][is_index] != NULL) {
							if (is_index == s_index) {
								if ((int)this->scope_context.size()+1 < this->new_state_furthest_layer_needed_in[s_index]) {
									this->new_state_furthest_layer_needed_in[s_index] = (int)this->scope_context.size()+1;
								}
							} else {
								double sum_state_impact = 0.0;
								for (int in_index = 0; in_index < 20; in_index++) {
									if (abs(this->step_state_networks[a_index][is_index]->output->weights[0][0][in_index]) > 0.05) {
										sum_state_impact += abs(this->step_state_networks[a_index][is_index]->hidden->weights[in_index][2][s_index]);
									}
								}

								if (sum_state_impact > 0.1) {
									if ((int)this->scope_context.size()+1 < this->new_state_furthest_layer_needed_in[s_index]) {
										this->new_state_furthest_layer_needed_in[s_index] = (int)this->scope_context.size()+1;
									}
								}
							}
						}
					}

					{
						double sum_state_impact = 0.0;
						for (int in_index = 0; in_index < 20; in_index++) {
							sum_state_impact += abs(this->step_score_networks[a_index]->hidden->weights[in_index][1][s_index]);
						}

						if (sum_state_impact > 0.1) {
							if ((int)this->scope_context.size()+1 < this->new_state_furthest_layer_needed_in[s_index]) {
								this->new_state_furthest_layer_needed_in[s_index] = (int)this->scope_context.size()+1;
							}
						}
					}
				}
			}

			{
				double sum_state_impact = 0.0;
				for (int in_index = 0; in_index < 20; in_index++) {
					sum_state_impact += abs(this->starting_score_network->hidden->weights[in_index][1][s_index]);
				}

				if (sum_state_impact > 0.1) {
					if ((int)this->scope_context.size() < this->new_state_furthest_layer_needed_in[s_index]) {
						this->new_state_furthest_layer_needed_in[s_index] = (int)this->scope_context.size();
					}
				}
			}

			{
				double sum_state_impact = 0.0;
				for (int in_index = 0; in_index < 20; in_index++) {
					sum_state_impact += abs(this->starting_misguess_network->hidden->weights[in_index][1][s_index]);
				}

				if (sum_state_impact > 0.1) {
					if ((int)this->scope_context.size() < this->new_state_furthest_layer_needed_in[s_index]) {
						this->new_state_furthest_layer_needed_in[s_index] = (int)this->scope_context.size();
					}
				}
			}

			for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
				if (this->exit_networks[e_index] != NULL) {
					double sum_state_impact = 0.0;
					for (int in_index = 0; in_index < 20; in_index++) {
						if (abs(this->exit_networks[e_index]->output->weights[0][0][in_index]) > 0.05) {
							sum_state_impact += abs(this->exit_networks[e_index]->hidden->weights[in_index][1][s_index]);
						}
					}

					if (sum_state_impact > 0.1) {
						if ((int)this->scope_context.size() < this->new_state_furthest_layer_needed_in[s_index]) {
							this->new_state_furthest_layer_needed_in[s_index] = (int)this->scope_context.size();
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
		for (int l_index = 0; l_index < (int)this->scope_context.size()+2; l_index++) {
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
			int furthest_layer_seen_in = this->scope_furthest_layer_seen_in[it->first];
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

		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
				for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
					if (this->step_state_networks[a_index][s_index] != NULL) {
						this->step_state_networks[a_index][s_index]->clean(this->layer_num_new_states.back());
					}
				}
			}
		}

		for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
			if (this->exit_networks[e_index] != NULL) {
				this->exit_networks[e_index]->clean(this->layer_num_new_states[this->scope_context.size()]);
			}
		}

		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
				int input_size = (int)this->sequences[a_index]->input_types.size();
				for (int i_index = 0; i_index < input_size; i_index++) {
					if (!this->sequences[a_index]->input_is_new_class[i_index]) {
						// includes SEQUENCE_INPUT_TYPE_NONE and SEQUENCE_INPUT_TYPE_LAST_SEEN
						for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
								it != this->sequences[a_index]->state_networks.end(); it++) {
							int furthest_layer_seen_in = this->scope_furthest_layer_seen_in[it->first];
							int num_new_states = this->layer_num_new_states[furthest_layer_seen_in];

							for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
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

		this->state = EXPERIMENT_STATE_FIRST_CLEAN;
		this->state_iter = 0;
		this->sum_error = 0.0;
	} else {
		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
				it != this->state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					delete it->second[n_index][s_index];
				}
			}
		}

		for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
				it != this->score_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				if (it->second[n_index] != NULL) {
					delete it->second[n_index];
				}
			}
		}

		delete this->starting_score_network;
		delete this->starting_misguess_network;

		for (int a_index = 0; a_index < this->num_steps; a_index++) {
			if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
				for (int s_index = 0; s_index < (int)this->step_state_networks[a_index].size(); s_index++) {
					delete this->step_state_networks[a_index][s_index];
				}

				delete this->step_score_networks[a_index];
			} else {
				for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
						it != this->sequences[a_index]->state_networks.end(); it++) {
					for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
						for (int i_index = 0; i_index < (int)it->second[n_index].size(); i_index++) {
							delete it->second[n_index][i_index];
						}
					}
				}

				for (int ia_index = 0; ia_index < (int)this->sequences[a_index]->step_state_networks.size(); ia_index++) {
					for (int i_index = 0; i_index < (int)this->sequences[a_index]->step_state_networks[ia_index].size(); i_index++) {
						delete this->sequences[a_index]->step_state_networks[ia_index][i_index];
					}
				}

				delete this->sequences[a_index];

				delete this->sequence_scale_mods[a_index];
			}
		}

		for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
			delete this->exit_networks[e_index];
		}

		delete this->seed_context_history;

		this->state = EXPERIMENT_STATE_DONE;
	}
}
