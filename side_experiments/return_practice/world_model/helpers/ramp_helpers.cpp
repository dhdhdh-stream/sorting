#include "world_model_helpers.h"

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "world_model_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int RAMP_TRAIN_ITERS = 20;
#else
const int RAMP_TRAIN_ITERS = 200000;
#endif /* MDEBUG */

const int NUM_NEW_NETWORKS = 2;

void ramp_helper(vector<Network*>& temp_obs_networks,
				 vector<double>& temp_obs_network_means,
				 vector<double>& temp_obs_network_diffs,
				 vector<Network*>& temp_action_networks,
				 vector<double>& temp_action_network_means,
				 vector<double>& temp_action_network_diffs,
				 Network* new_final_network,
				 WorldModel* potential_world_model,
				 WorldModelWrapper* wrapper) {
	vector<Network*> new_obs_networks;
	for (int n_index = 0; n_index < NUM_NEW_NETWORKS; n_index++) {
		new_obs_networks.push_back(new Network(potential_world_model->num_states + wrapper->num_obs,
			temp_obs_networks.size() + temp_action_networks.size()));
	}

	vector<Network*> new_action_networks;
	for (int n_index = 0; n_index < NUM_NEW_NETWORKS; n_index++) {
		new_action_networks.push_back(new Network(potential_world_model->num_states + wrapper->num_actions,
			temp_obs_networks.size() + temp_action_networks.size()));
	}

	uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);
	for (int iter_index = 0; iter_index < RAMP_TRAIN_ITERS; iter_index++) {
		int sample_index = sample_distribution(generator);

		uniform_int_distribution<int> include_obs_distribution(
			0, wrapper->sample_obs[sample_index].size()-1);
		int include_obs_index = include_obs_distribution(generator);

		vector<double> state(potential_world_model->num_states, 0.0);
		vector<double> new_state(temp_obs_networks.size() + temp_action_networks.size(), 0.0);

		vector<vector<NetworkHistory*>> obs_network_histories;
		vector<vector<NetworkHistory*>> new_obs_network_histories;
		vector<vector<NetworkHistory*>> action_network_histories;
		vector<vector<NetworkHistory*>> new_action_network_histories;

		double ratio = 1.0 - (double)iter_index / (double)RAMP_TRAIN_ITERS;

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

				vector<double> temp_inputs;
				temp_inputs.insert(temp_inputs.end(), starting_state.begin(), starting_state.end());
				temp_inputs.insert(temp_inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
					wrapper->sample_obs[sample_index][step_index].end());
				for (int n_index = 0; n_index < (int)temp_obs_networks.size(); n_index++) {
					temp_obs_networks[n_index]->activate(temp_inputs);
					double normalized = (temp_obs_networks[n_index]->output->acti_vals[0]
						- temp_obs_network_means[n_index]) / temp_obs_network_diffs[n_index];
					new_state[n_index] += ratio * normalized;
				}

				vector<NetworkHistory*> step_new_obs_network_histories;
				for (int n_index = 0; n_index < (int)new_obs_networks.size(); n_index++) {
					vector<double> inputs = starting_state;
					inputs.insert(inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
						wrapper->sample_obs[sample_index][step_index].end());
					NetworkHistory* network_history = new NetworkHistory();
					new_obs_networks[n_index]->activate(inputs,
														network_history);
					step_new_obs_network_histories.push_back(network_history);
					for (int o_index = 0; o_index < (int)new_state.size(); o_index++) {
						new_state[o_index] += new_obs_networks[n_index]->output->acti_vals[o_index];
					}
				}
				new_obs_network_histories.push_back(step_new_obs_network_histories);
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

				vector<double> temp_inputs;
				temp_inputs.insert(temp_inputs.end(), starting_state.begin(), starting_state.end());
				temp_inputs.insert(temp_inputs.end(), partial_inputs.begin(), partial_inputs.end());
				for (int n_index = 0; n_index < (int)temp_action_networks.size(); n_index++) {
					temp_action_networks[n_index]->activate(temp_inputs);
					double normalized = (temp_action_networks[n_index]->output->acti_vals[0]
						- temp_action_network_means[n_index]) / temp_action_network_diffs[n_index];
					new_state[(int)temp_obs_networks.size() + n_index] += ratio * normalized;
				}

				vector<NetworkHistory*> step_new_action_network_histories;
				for (int n_index = 0; n_index < (int)new_action_networks.size(); n_index++) {
					vector<double> inputs = starting_state;
					inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
					NetworkHistory* network_history = new NetworkHistory();
					new_action_networks[n_index]->activate(inputs,
														   network_history);
					step_new_action_network_histories.push_back(network_history);
					for (int o_index = 0; o_index < (int)new_state.size(); o_index++) {
						new_state[o_index] += new_action_networks[n_index]->output->acti_vals[o_index];
					}
				}
				new_action_network_histories.push_back(step_new_action_network_histories);
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

		vector<double> new_final_inputs;
		new_final_inputs.insert(new_final_inputs.end(), state.begin(), state.end());
		new_final_inputs.insert(new_final_inputs.end(), new_state.begin(), new_state.end());
		new_final_network->activate(new_final_inputs);
		sum_score += new_final_network->output->acti_vals[0];

		vector<double> final_errors{wrapper->sample_target_vals[sample_index] - sum_score};

		vector<double> state_errors(potential_world_model->num_states, 0.0);
		vector<double> new_state_errors(temp_obs_networks.size() + temp_action_networks.size(), 0.0);

		for (int n_index = 0; n_index < (int)potential_world_model->final_networks.size(); n_index++) {
			potential_world_model->final_networks[n_index]->backprop(final_errors);
			for (int i_index = 0; i_index < (int)potential_world_model->final_network_inputs[n_index].size(); i_index++) {
				state_errors[potential_world_model->final_network_inputs[n_index][i_index]]
					+= potential_world_model->final_networks[n_index]->input->errors[i_index];
				potential_world_model->final_networks[n_index]->input->errors[i_index] = 0.0;
			}
		}

		new_final_network->backprop(final_errors);
		for (int i_index = 0; i_index < (int)state_errors.size(); i_index++) {
			state_errors[i_index] += new_final_network->input->errors[i_index];
			new_final_network->input->errors[i_index] = 0.0;
		}
		for (int i_index = 0; i_index < (int)new_state_errors.size(); i_index++) {
			new_state_errors[i_index] += new_final_network->input->errors[state_errors.size() + i_index];
			new_final_network->input->errors[state_errors.size() + i_index] = 0.0;
		}

		for (int step_index = (int)wrapper->sample_obs[sample_index].size()-1; step_index >= 0; step_index--) {
			if (step_index < (int)wrapper->sample_actions[sample_index].size()) {
				vector<double> starting_errors = state_errors;

				for (int n_index = 0; n_index < (int)potential_world_model->action_networks.size(); n_index++) {
					vector<double> errors;
					for (int o_index = 0; o_index < (int)potential_world_model->action_network_outputs[n_index].size(); o_index++) {
						errors.push_back(starting_errors[potential_world_model->action_network_inputs[n_index][o_index]]);
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

				for (int n_index = 0; n_index < (int)new_action_networks.size(); n_index++) {
					vector<double> errors = new_state_errors;
					new_action_networks[n_index]->backprop(errors,
														   new_action_network_histories[step_index][n_index]);
					delete new_action_network_histories[step_index][n_index];
					for (int i_index = 0; i_index < (int)state_errors.size(); i_index++) {
						state_errors[i_index] += new_action_networks[n_index]->input->errors[i_index];
						new_action_networks[n_index]->input->errors[i_index] = 0.0;
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

				for (int n_index = 0; n_index < (int)new_obs_networks.size(); n_index++) {
					vector<double> errors = new_state_errors;
					new_obs_networks[n_index]->backprop(errors,
														new_obs_network_histories[step_index][n_index]);
					delete new_obs_network_histories[step_index][n_index];
					for (int i_index = 0; i_index < (int)state_errors.size(); i_index++) {
						state_errors[i_index] += new_obs_networks[n_index]->input->errors[i_index];
						new_obs_networks[n_index]->input->errors[i_index] = 0.0;
					}
				}
			}
		}

		if ((iter_index + 1)%NETWORK_EPOCH_SIZE == 0) {
			for (int n_index = 0; n_index < (int)new_obs_networks.size(); n_index++) {
				new_obs_networks[n_index]->update();
			}
			for (int n_index = 0; n_index < (int)new_action_networks.size(); n_index++) {
				new_action_networks[n_index]->update();
			}
			new_final_network->update();

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

	vector<int> new_network_inputs;
	for (int i_index = 0; i_index < potential_world_model->num_states; i_index++) {
		new_network_inputs.push_back(i_index);
	}
	vector<int> new_network_outputs;
	for (int o_index = 0; o_index < (int)temp_obs_networks.size() + (int)temp_action_networks.size(); o_index++) {
		new_network_outputs.push_back(potential_world_model->num_states + o_index);
	}

	potential_world_model->num_states += (int)temp_obs_networks.size() + (int)temp_action_networks.size();

	vector<int> new_final_network_inputs;
	for (int i_index = 0; i_index < potential_world_model->num_states; i_index++) {
		new_final_network_inputs.push_back(i_index);
	}

	for (int n_index = 0; (int)n_index < (int)new_obs_networks.size(); n_index++) {
		potential_world_model->obs_network_inputs.push_back(new_network_inputs);
		potential_world_model->obs_network_outputs.push_back(new_network_outputs);
		potential_world_model->obs_networks.push_back(new_obs_networks[n_index]);
	}

	for (int n_index = 0; (int)n_index < (int)new_action_networks.size(); n_index++) {
		potential_world_model->action_network_inputs.push_back(new_network_inputs);
		potential_world_model->action_network_outputs.push_back(new_network_outputs);
		potential_world_model->action_networks.push_back(new_action_networks[n_index]);
	}

	potential_world_model->final_network_inputs.push_back(new_final_network_inputs);
	potential_world_model->final_networks.push_back(new_final_network);
}
