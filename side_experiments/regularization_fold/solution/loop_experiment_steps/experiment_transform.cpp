#include "loop_experiment.h"

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

void LoopExperiment::experiment_transform() {
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
	if (score_improvement_t_value > 2.326) {	// >99%
		is_success = true;
	} else if (score_improvement_t_value > -0.674	// 75%<
			&& misguess_improvement_t_value > 2.326) {
		is_success = true;
	} else {
		is_success = false;
	}

	if (is_success) {
		int input_size = (int)this->sequence->input_types.size();

		// determine if new class needed
		this->sequence->input_furthest_layer_needed_in = vector<int>(input_size, (int)this->scope_context.size()+2);	// 1 greater than lowest
		this->sequence->input_is_new_class = vector<bool>(input_size, false);
		for (int i_index = 0; i_index < input_size; i_index++) {
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequence->state_networks.begin();
					it != this->sequence->state_networks.end(); it++) {
				int furthest_layer_seen_in = this->scope_furthest_layer_seen_in[it->first];

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

								for (int s_index = 0; s_index < (int)network->new_state_indexes.size(); s_index++) {
									sum_impact += abs(network->hidden->weights[in_index][2][s_index]);
								}
							}
						}

						if (sum_impact > 0.1) {
							// network needed
							if (furthest_layer_seen_in < this->sequence->input_furthest_layer_needed_in[i_index]) {
								this->sequence->input_furthest_layer_needed_in[i_index] = furthest_layer_seen_in;
							}
						}
					}
				}
			}

			// this->sequence->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL
			if (this->sequence->input_furthest_layer_needed_in[i_index]
					<= (int)this->scope_context.size() - this->sequence->input_local_scope_depths[i_index]) {
				this->sequence->input_is_new_class[i_index] = true;
			}
		}

		// remove networks if not new class
		this->sequence->scope_additions_needed = vector<set<int>>(input_size);
		for (int i_index = 0; i_index < input_size; i_index++) {
			if (!this->sequence->input_is_new_class[i_index]) {
				for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequence->state_networks.begin();
						it != this->sequence->state_networks.end(); it++) {
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

									for (int s_index = 0; s_index < (int)network->new_state_indexes.size(); s_index++) {
										sum_impact += abs(network->hidden->weights[in_index][2][s_index]);
									}
								}
							}

							if (sum_impact > 0.1) {
								this->sequence->scope_additions_needed[i_index].insert(it->first);
								cout << "keep input " << it->first << " " << n_index << " " << i_index << endl;
							} else {
								delete it->second[n_index][i_index];
								it->second[n_index][i_index] = NULL;
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

								for (int is_index = 0; is_index < (int)network->new_state_indexes.size(); is_index++) {
									sum_impact += abs(network->hidden->weights[in_index][2][is_index]);
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
		}

		for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
			if (this->exit_network_impacts[e_index] < 0.1) {
				delete this->exit_networks[e_index];
				this->exit_networks[e_index] = NULL;
			} else {
				cout << "keep exit " << e_index << endl;
			}
		}

		// determine new state layers needed in
		this->new_state_furthest_layer_needed_in = vector<int>(NUM_NEW_STATES, (int)this->scope_context.size()+2);
		this->new_state_steps_needed_in = vector<vector<bool>>(NUM_NEW_STATES, vector<bool>(1, false));
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

									if (steps_seen_in[0]) {
										this->new_state_steps_needed_in[s_index][0] = true;
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

										if (steps_seen_in[0]) {
											this->new_state_steps_needed_in[s_index][0] = true;
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
							cout << it->first << " " << n_index << " score needed " << s_index << endl;

							if (furthest_layer_seen_in < this->new_state_furthest_layer_needed_in[s_index]) {
								this->new_state_furthest_layer_needed_in[s_index] = furthest_layer_seen_in;
							}

							if (steps_seen_in[0]) {
								this->new_state_steps_needed_in[s_index][0] = true;
							}
						}
					}
				}
			}

			{
				double sum_state_impact = 0.0;
				for (int in_index = 0; in_index < 20; in_index++) {
					sum_state_impact += abs(this->continue_score_network->hidden->weights[in_index][1][s_index]);
				}

				if (sum_state_impact > 0.1) {
					if ((int)this->scope_context.size()+1 < this->new_state_furthest_layer_needed_in[s_index]) {
						this->new_state_furthest_layer_needed_in[s_index] = (int)this->scope_context.size()+1;
					}
				}
			}

			{
				double sum_state_impact = 0.0;
				for (int in_index = 0; in_index < 20; in_index++) {
					sum_state_impact += abs(this->continue_misguess_network->hidden->weights[in_index][1][s_index]);
				}

				if (sum_state_impact > 0.1) {
					if ((int)this->scope_context.size()+1 < this->new_state_furthest_layer_needed_in[s_index]) {
						this->new_state_furthest_layer_needed_in[s_index] = (int)this->scope_context.size()+1;
					}
				}
			}

			{
				double sum_state_impact = 0.0;
				for (int in_index = 0; in_index < 20; in_index++) {
					sum_state_impact += abs(this->halt_score_network->hidden->weights[in_index][1][s_index]);
				}

				if (sum_state_impact > 0.1) {
					if ((int)this->scope_context.size()+1 < this->new_state_furthest_layer_needed_in[s_index]) {
						this->new_state_furthest_layer_needed_in[s_index] = (int)this->scope_context.size()+1;
					}
				}
			}

			{
				double sum_state_impact = 0.0;
				for (int in_index = 0; in_index < 20; in_index++) {
					sum_state_impact += abs(this->halt_misguess_network->hidden->weights[in_index][1][s_index]);
				}

				if (sum_state_impact > 0.1) {
					if ((int)this->scope_context.size()+1 < this->new_state_furthest_layer_needed_in[s_index]) {
						this->new_state_furthest_layer_needed_in[s_index] = (int)this->scope_context.size()+1;
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

		for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
			if (this->exit_networks[e_index] != NULL) {
				this->exit_networks[e_index]->clean(this->layer_num_new_states[this->scope_context.size()]);
			}
		}

		for (int i_index = 0; i_index < input_size; i_index++) {
			if (!this->sequence->input_is_new_class[i_index]) {
				for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequence->state_networks.begin();
						it != this->sequence->state_networks.end(); it++) {
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

		delete this->continue_score_network;
		delete this->continue_misguess_network;
		delete this->halt_score_network;
		delete this->halt_misguess_network;

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequence->state_networks.begin();
				it != this->sequence->state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int i_index = 0; i_index < (int)it->second[n_index].size(); i_index++) {
					delete it->second[n_index][i_index];
				}
			}
		}
		delete this->sequence;

		delete this->scale_mod;

		for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
			delete this->exit_networks[e_index];
		}

		this->state = EXPERIMENT_STATE_DONE;
	}
}
