#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "world_model_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int FINALIZE_TRAIN_ITERS = 10;
#else
const int FINALIZE_TRAIN_ITERS = 100000;
#endif /* MDEBUG */

void finalize_helper(WorldModel* potential_world_model,
					 WorldModelWrapper* wrapper) {
	// temp
	cout << "finalize_helper" << endl;

	uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);
	for (int iter_index = 0; iter_index < FINALIZE_TRAIN_ITERS; iter_index++) {
		int sample_index = sample_distribution(generator);

		uniform_int_distribution<int> include_obs_distribution(
			0, wrapper->sample_obs[sample_index].size()-1);
		int include_obs_index = include_obs_distribution(generator);

		vector<double> state(potential_world_model->num_states, 0.0);

		vector<vector<NetworkHistory*>> obs_network_histories;
		vector<vector<NetworkHistory*>> action_network_histories;

		for (int step_index = 0; step_index < (int)wrapper->sample_obs[sample_index].size(); step_index++) {
			if (step_index <= include_obs_index) {
				vector<double> starting_state = state;

				vector<NetworkHistory*> step_obs_network_histories;
				for (int n_index = 0; n_index < (int)potential_world_model->obs_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)potential_world_model->obs_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[potential_world_model->obs_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
						wrapper->sample_obs[sample_index][step_index].end());
					NetworkHistory* network_history = new NetworkHistory();
					potential_world_model->obs_networks[n_index]->activate(inputs,
																		   network_history);
					step_obs_network_histories.push_back(network_history);
					for (int o_index = 0; o_index < (int)potential_world_model->obs_network_outputs[n_index].size(); o_index++) {
						state[potential_world_model->obs_network_outputs[n_index][o_index]]
							+= potential_world_model->obs_networks[n_index]->output->acti_vals[o_index];
					}
				}
				obs_network_histories.push_back(step_obs_network_histories);
			}

			if (step_index < (int)wrapper->sample_actions[sample_index].size()) {
				int action = wrapper->sample_actions[sample_index][step_index];

				vector<double> starting_state = state;

				vector<double> partial_inputs;
				for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
					if (action == a_index) {
						partial_inputs.push_back(1.0);
					} else {
						partial_inputs.push_back(0.0);
					}
				}

				vector<NetworkHistory*> step_action_network_histories;
				for (int n_index = 0; n_index < (int)potential_world_model->action_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)potential_world_model->action_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[potential_world_model->action_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
					NetworkHistory* network_history = new NetworkHistory();
					potential_world_model->action_networks[n_index]->activate(inputs,
																			  network_history);
					step_action_network_histories.push_back(network_history);
					for (int o_index = 0; o_index < (int)potential_world_model->action_network_outputs[n_index].size(); o_index++) {
						state[potential_world_model->action_network_outputs[n_index][o_index]]
							+= potential_world_model->action_networks[n_index]->output->acti_vals[o_index];
					}
				}
				action_network_histories.push_back(step_action_network_histories);
			}
		}

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)potential_world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)potential_world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[potential_world_model->final_network_inputs[n_index][i_index]]);
			}
			potential_world_model->final_networks[n_index]->activate(inputs);
			sum_score += potential_world_model->final_networks[n_index]->output->acti_vals[0];
		}

		vector<double> final_errors{wrapper->sample_target_vals[sample_index] - sum_score};

		vector<double> state_errors(potential_world_model->num_states, 0.0);

		for (int n_index = 0; n_index < (int)potential_world_model->final_networks.size(); n_index++) {
			potential_world_model->final_networks[n_index]->backprop(final_errors);
			for (int i_index = 0; i_index < (int)potential_world_model->final_network_inputs[n_index].size(); i_index++) {
				state_errors[potential_world_model->final_network_inputs[n_index][i_index]]
					+= potential_world_model->final_networks[n_index]->input->errors[i_index];
				potential_world_model->final_networks[n_index]->input->errors[i_index] = 0.0;
			}
		}

		for (int step_index = (int)wrapper->sample_obs[sample_index].size()-1; step_index >= 0; step_index--) {
			if (step_index < (int)wrapper->sample_actions[sample_index].size()) {
				vector<double> starting_errors = state_errors;

				for (int n_index = 0; n_index < (int)potential_world_model->action_networks.size(); n_index++) {
					vector<double> errors;
					for (int o_index = 0; o_index < (int)potential_world_model->action_network_outputs[n_index].size(); o_index++) {
						errors.push_back(starting_errors[potential_world_model->action_network_outputs[n_index][o_index]]);
					}
					potential_world_model->action_networks[n_index]->backprop(errors,
																			  action_network_histories[step_index][n_index]);
					delete action_network_histories[step_index][n_index];
					for (int i_index = 0; i_index < (int)potential_world_model->action_network_inputs[n_index].size(); i_index++) {
						state_errors[potential_world_model->action_network_inputs[n_index][i_index]]
							+= potential_world_model->action_networks[n_index]->input->errors[i_index];
						potential_world_model->action_networks[n_index]->input->errors[i_index] = 0.0;
					}
				}
			}

			if (step_index <= include_obs_index) {
				vector<double> starting_errors = state_errors;

				for (int n_index = 0; n_index < (int)potential_world_model->obs_networks.size(); n_index++) {
					vector<double> errors;
					for (int o_index = 0; o_index < (int)potential_world_model->obs_network_outputs[n_index].size(); o_index++) {
						errors.push_back(starting_errors[potential_world_model->obs_network_outputs[n_index][o_index]]);
					}
					potential_world_model->obs_networks[n_index]->backprop(errors,
																		   obs_network_histories[step_index][n_index]);
					delete obs_network_histories[step_index][n_index];
					for (int i_index = 0; i_index < (int)potential_world_model->obs_network_inputs[n_index].size(); i_index++) {
						state_errors[potential_world_model->obs_network_inputs[n_index][i_index]]
							+= potential_world_model->obs_networks[n_index]->input->errors[i_index];
						potential_world_model->obs_networks[n_index]->input->errors[i_index] = 0.0;
					}
				}
			}
		}

		if ((iter_index + 1)%NETWORK_EPOCH_SIZE == 0) {
			double max_update = 0.0;
			for (int n_index = 0; n_index < (int)potential_world_model->obs_networks.size(); n_index++) {
				potential_world_model->obs_networks[n_index]->get_max_update(max_update);
			}
			for (int n_index = 0; n_index < (int)potential_world_model->action_networks.size(); n_index++) {
				potential_world_model->action_networks[n_index]->get_max_update(max_update);
			}
			for (int n_index = 0; n_index < (int)potential_world_model->final_networks.size(); n_index++) {
				potential_world_model->final_networks[n_index]->get_max_update(max_update);
			}

			potential_world_model->average_max_update += 0.999*potential_world_model->average_max_update + 0.001*max_update;

			if (max_update > 0.0) {
				double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / potential_world_model->average_max_update;
				if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
					learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
				}

				for (int n_index = 0; n_index < (int)potential_world_model->obs_networks.size(); n_index++) {
					potential_world_model->obs_networks[n_index]->update_weights(learning_rate);
				}
				for (int n_index = 0; n_index < (int)potential_world_model->action_networks.size(); n_index++) {
					potential_world_model->action_networks[n_index]->update_weights(learning_rate);
				}
				for (int n_index = 0; n_index < (int)potential_world_model->final_networks.size(); n_index++) {
					potential_world_model->final_networks[n_index]->update_weights(learning_rate);
				}
			}
		}
	}

	// // temp
	// for (int iter_index = 0; iter_index < 20; iter_index++) {
	// 	cout << iter_index << endl;

	// 	int sample_index = sample_distribution(generator);

	// 	uniform_int_distribution<int> include_obs_distribution(
	// 		0, wrapper->sample_obs[sample_index].size()-1);
	// 	int include_obs_index = include_obs_distribution(generator);

	// 	vector<double> state(potential_world_model->num_states, 0.0);

	// 	for (int step_index = 0; step_index < (int)wrapper->sample_obs[sample_index].size(); step_index++) {
	// 		cout << "step_index: " << step_index << endl;

	// 		if (step_index <= include_obs_index) {
	// 			cout << "obs:";
	// 			for (int o_index = 0; o_index < (int)wrapper->sample_obs[sample_index][step_index].size(); o_index++) {
	// 				cout << " " << wrapper->sample_obs[sample_index][step_index][o_index];
	// 			}
	// 			cout << endl;

	// 			vector<double> starting_state = state;

	// 			for (int n_index = 0; n_index < (int)potential_world_model->obs_networks.size(); n_index++) {
	// 				vector<double> inputs;
	// 				for (int i_index = 0; i_index < (int)potential_world_model->obs_network_inputs[n_index].size(); i_index++) {
	// 					inputs.push_back(starting_state[potential_world_model->obs_network_inputs[n_index][i_index]]);
	// 				}
	// 				inputs.insert(inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
	// 					wrapper->sample_obs[sample_index][step_index].end());
	// 				potential_world_model->obs_networks[n_index]->activate(inputs);
	// 				for (int o_index = 0; o_index < (int)potential_world_model->obs_network_outputs[n_index].size(); o_index++) {
	// 					state[potential_world_model->obs_network_outputs[n_index][o_index]]
	// 						+= potential_world_model->obs_networks[n_index]->output->acti_vals[o_index];
	// 				}
	// 			}

	// 			cout << "state:";
	// 			for (int s_index = 0; s_index < (int)state.size(); s_index++) {
	// 				cout << " " << state[s_index];
	// 			}
	// 			cout << endl;
	// 		}

	// 		if (step_index < (int)wrapper->sample_actions[sample_index].size()) {
	// 			int action = wrapper->sample_actions[sample_index][step_index];
	// 			cout << "action: " << action << endl;

	// 			vector<double> starting_state = state;

	// 			vector<double> partial_inputs;
	// 			for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
	// 				if (action == a_index) {
	// 					partial_inputs.push_back(1.0);
	// 				} else {
	// 					partial_inputs.push_back(0.0);
	// 				}
	// 			}

	// 			for (int n_index = 0; n_index < (int)potential_world_model->action_networks.size(); n_index++) {
	// 				vector<double> inputs;
	// 				for (int i_index = 0; i_index < (int)potential_world_model->action_network_inputs[n_index].size(); i_index++) {
	// 					inputs.push_back(starting_state[potential_world_model->action_network_inputs[n_index][i_index]]);
	// 				}
	// 				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
	// 				potential_world_model->action_networks[n_index]->activate(inputs);
	// 				for (int o_index = 0; o_index < (int)potential_world_model->action_network_outputs[n_index].size(); o_index++) {
	// 					state[potential_world_model->action_network_outputs[n_index][o_index]]
	// 						+= potential_world_model->action_networks[n_index]->output->acti_vals[o_index];
	// 				}
	// 			}

	// 			cout << "state:";
	// 			for (int s_index = 0; s_index < (int)state.size(); s_index++) {
	// 				cout << " " << state[s_index];
	// 			}
	// 			cout << endl;
	// 		}
	// 	}

	// 	double sum_score = 0.0;
	// 	for (int n_index = 0; n_index < (int)potential_world_model->final_networks.size(); n_index++) {
	// 		vector<double> inputs;
	// 		for (int i_index = 0; i_index < (int)potential_world_model->final_network_inputs[n_index].size(); i_index++) {
	// 			inputs.push_back(state[potential_world_model->final_network_inputs[n_index][i_index]]);
	// 		}
	// 		potential_world_model->final_networks[n_index]->activate(inputs);
	// 		sum_score += potential_world_model->final_networks[n_index]->output->acti_vals[0];
	// 	}

	// 	cout << "sum_score: " << sum_score << endl;
	// 	cout << "wrapper->sample_target_vals[sample_index]: " << wrapper->sample_target_vals[sample_index] << endl;
	// }

	// temp
	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)wrapper->sample_obs.size(); h_index++) {
		uniform_int_distribution<int> include_obs_distribution(
			0, wrapper->sample_obs[h_index].size()-1);
		int include_obs_index = include_obs_distribution(generator);

		vector<double> state(potential_world_model->num_states, 0.0);

		for (int step_index = 0; step_index < (int)wrapper->sample_obs[h_index].size(); step_index++) {
			if (step_index <= include_obs_index) {
				vector<double> starting_state = state;

				for (int n_index = 0; n_index < (int)potential_world_model->obs_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)potential_world_model->obs_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[potential_world_model->obs_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), wrapper->sample_obs[h_index][step_index].begin(),
						wrapper->sample_obs[h_index][step_index].end());
					potential_world_model->obs_networks[n_index]->activate(inputs);
					for (int o_index = 0; o_index < (int)potential_world_model->obs_network_outputs[n_index].size(); o_index++) {
						state[potential_world_model->obs_network_outputs[n_index][o_index]]
							+= potential_world_model->obs_networks[n_index]->output->acti_vals[o_index];
					}
				}
			}

			if (step_index < (int)wrapper->sample_actions[h_index].size()) {
				int action = wrapper->sample_actions[h_index][step_index];

				vector<double> starting_state = state;

				vector<double> partial_inputs;
				for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
					if (action == a_index) {
						partial_inputs.push_back(1.0);
					} else {
						partial_inputs.push_back(0.0);
					}
				}

				for (int n_index = 0; n_index < (int)potential_world_model->action_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)potential_world_model->action_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[potential_world_model->action_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
					potential_world_model->action_networks[n_index]->activate(inputs);
					for (int o_index = 0; o_index < (int)potential_world_model->action_network_outputs[n_index].size(); o_index++) {
						state[potential_world_model->action_network_outputs[n_index][o_index]]
							+= potential_world_model->action_networks[n_index]->output->acti_vals[o_index];
					}
				}
			}
		}

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)potential_world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)potential_world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[potential_world_model->final_network_inputs[n_index][i_index]]);
			}
			potential_world_model->final_networks[n_index]->activate(inputs);
			sum_score += potential_world_model->final_networks[n_index]->output->acti_vals[0];
		}

		sum_misguess += (wrapper->sample_target_vals[h_index] - sum_score)
			* (wrapper->sample_target_vals[h_index] - sum_score);
	}
	double misguess_average = sum_misguess / (double)wrapper->sample_obs.size();
	cout << "misguess_average: " << misguess_average << endl;
}
