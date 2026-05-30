// - once state has been created, do not change?
//   - at least if the goal is to "memorize" tried sequences

#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int UPDATE_NUM_ITERS = 2;
#else
const int UPDATE_NUM_ITERS = 5;
#endif /* MDEBUG */

void update_world_model_helper(vector<vector<double>>& obs,
							   vector<int>& actions,
							   double target_val,
							   Wrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

	uniform_int_distribution<int> include_obs_distribution(0, obs.size()-1);
	for (int iter_index = 0; iter_index < UPDATE_NUM_ITERS; iter_index++) {
		int include_obs_index = include_obs_distribution(generator);

		vector<double> state(world_model->num_states, 0.0);

		vector<vector<NetworkHistory*>> obs_network_histories;
		vector<vector<NetworkHistory*>> action_network_histories;

		for (int step_index = 0; step_index < (int)obs.size(); step_index++) {
			if (step_index <= include_obs_index) {
				vector<double> starting_state = state;

				vector<NetworkHistory*> step_obs_network_histories;
				for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), obs[step_index].begin(),
						obs[step_index].end());
					NetworkHistory* network_history = new NetworkHistory();
					world_model->obs_networks[n_index]->activate(inputs,
																 network_history);
					step_obs_network_histories.push_back(network_history);
					for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
						state[world_model->obs_network_outputs[n_index][o_index]]
							+= world_model->obs_networks[n_index]->output->acti_vals[o_index];
					}
				}
				obs_network_histories.push_back(step_obs_network_histories);
			}

			if (step_index < (int)actions.size()) {
				int action = actions[step_index];

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
				for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)world_model->action_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[world_model->action_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
					NetworkHistory* network_history = new NetworkHistory();
					world_model->action_networks[n_index]->activate(inputs,
																	network_history);
					step_action_network_histories.push_back(network_history);
					for (int o_index = 0; o_index < (int)world_model->action_network_outputs[n_index].size(); o_index++) {
						state[world_model->action_network_outputs[n_index][o_index]]
							+= world_model->action_networks[n_index]->output->acti_vals[o_index];
					}
				}
				action_network_histories.push_back(step_action_network_histories);
			}
		}

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[world_model->final_network_inputs[n_index][i_index]]);
			}
			world_model->final_networks[n_index]->activate(inputs);
			sum_score += world_model->final_networks[n_index]->output->acti_vals[0];
		}

		vector<double> final_errors{target_val - sum_score};

		vector<double> state_errors(world_model->num_states, 0.0);

		for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
			world_model->final_networks[n_index]->backprop(final_errors);
			for (int i_index = 0; i_index < (int)world_model->final_network_inputs[n_index].size(); i_index++) {
				state_errors[world_model->final_network_inputs[n_index][i_index]]
					+= world_model->final_networks[n_index]->input->errors[i_index];
				world_model->final_networks[n_index]->input->errors[i_index] = 0.0;
			}
		}

		// for (int step_index = (int)obs.size()-1; step_index >= 0; step_index--) {
		// 	if (step_index < (int)actions.size()) {
		// 		vector<double> starting_errors = state_errors;

		// 		for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
		// 			vector<double> errors;
		// 			for (int o_index = 0; o_index < (int)world_model->action_network_outputs[n_index].size(); o_index++) {
		// 				errors.push_back(starting_errors[world_model->action_network_outputs[n_index][o_index]]);
		// 			}
		// 			world_model->action_networks[n_index]->backprop(errors,
		// 															action_network_histories[step_index][n_index]);
		// 			delete action_network_histories[step_index][n_index];
		// 			for (int i_index = 0; i_index < (int)world_model->action_network_inputs[n_index].size(); i_index++) {
		// 				state_errors[world_model->action_network_inputs[n_index][i_index]]
		// 					+= world_model->action_networks[n_index]->input->errors[i_index];
		// 				world_model->action_networks[n_index]->input->errors[i_index] = 0.0;
		// 			}
		// 		}
		// 	}

		// 	if (step_index <= include_obs_index) {
		// 		vector<double> starting_errors = state_errors;

		// 		for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
		// 			vector<double> errors;
		// 			for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
		// 				errors.push_back(starting_errors[world_model->obs_network_outputs[n_index][o_index]]);
		// 			}
		// 			world_model->obs_networks[n_index]->backprop(errors,
		// 														 obs_network_histories[step_index][n_index]);
		// 			delete obs_network_histories[step_index][n_index];
		// 			for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
		// 				state_errors[world_model->obs_network_inputs[n_index][i_index]]
		// 					+= world_model->obs_networks[n_index]->input->errors[i_index];
		// 				world_model->obs_networks[n_index]->input->errors[i_index] = 0.0;
		// 			}
		// 		}
		// 	}
		// }

		world_model->epoch_iter++;
		if (world_model->epoch_iter >= NETWORK_EPOCH_SIZE) {
			double max_update = 0.0;
			// for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
			// 	world_model->obs_networks[n_index]->get_max_update(max_update);
			// }
			// for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
			// 	world_model->action_networks[n_index]->get_max_update(max_update);
			// }
			for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
				world_model->final_networks[n_index]->get_max_update(max_update);
			}

			world_model->average_max_update = 0.999*world_model->average_max_update + 0.001*max_update;

			if (max_update > 0.0) {
				double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / world_model->average_max_update;
				if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
					learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
				}

				// for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				// 	world_model->obs_networks[n_index]->update_weights(learning_rate);
				// }
				// for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
				// 	world_model->action_networks[n_index]->update_weights(learning_rate);
				// }
				for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
					world_model->final_networks[n_index]->update_weights(learning_rate);
				}
			}

			world_model->epoch_iter = 0;
		}
	}
}
