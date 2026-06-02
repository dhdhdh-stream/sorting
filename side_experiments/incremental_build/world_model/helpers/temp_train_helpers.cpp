#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

const int NUM_EXISTING_STATES = 8;
const int NUM_NEW_STATES = 2;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

/**
 * - set train_target_vals outside
 */
void temp_train_helper(vector<vector<vector<double>>>& train_obs,
					   vector<vector<int>>& train_actions,
					   vector<double>& train_target_vals,
					   WorldModel* potential_world_model,
					   Wrapper* wrapper) {
	vector<int> new_obs_existing_inputs;
	{
		vector<int> remaining_indexes;
		for (int s_index = 0; s_index < potential_world_model->num_states; s_index++) {
			remaining_indexes.push_back(s_index);
		}

		while (remaining_indexes.size() > 0) {
			uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
			int index = distribution(generator);
			new_obs_existing_inputs.push_back(index);
			remaining_indexes.erase(remaining_indexes.begin() + index);

			if (new_obs_existing_inputs.size() >= NUM_EXISTING_STATES) {
				break;
			}
		}
	}
	Network* new_obs_network = new Network(NUM_NEW_STATES
		+ new_obs_existing_inputs.size() + wrapper->num_obs, NUM_NEW_STATES);

	vector<int> new_action_existing_inputs;
	{
		vector<int> remaining_indexes;
		for (int s_index = 0; s_index < potential_world_model->num_states; s_index++) {
			remaining_indexes.push_back(s_index);
		}

		while (remaining_indexes.size() > 0) {
			uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
			int index = distribution(generator);
			new_action_existing_inputs.push_back(index);
			remaining_indexes.erase(remaining_indexes.begin() + index);

			if (new_action_existing_inputs.size() >= NUM_EXISTING_STATES) {
				break;
			}
		}
	}
	Network* new_action_network = new Network(NUM_NEW_STATES
		+ new_action_existing_inputs.size() + wrapper->num_actions, NUM_NEW_STATES);

	Network* temp_final_network = new Network(NUM_NEW_STATES, 1);

	// temp
	double sum_error = 0.0;
	uniform_int_distribution<int> sample_distribution(0, train_obs.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int sample_index = sample_distribution(generator);

		uniform_int_distribution<int> include_obs_distribution(
			0, train_obs[sample_index].size()-1);
		// int include_obs_index = include_obs_distribution(generator);
		int include_obs_index = train_obs[sample_index].size()-1;

		vector<double> state(potential_world_model->num_states, 0.0);
		vector<double> new_state(NUM_NEW_STATES, 0.0);

		vector<NetworkHistory*> new_obs_network_histories;
		vector<NetworkHistory*> new_action_network_histories;

		for (int step_index = 0; step_index < (int)train_obs[sample_index].size(); step_index++) {
			if (step_index <= include_obs_index) {
				vector<double> starting_state = state;

				for (int n_index = 0; n_index < (int)potential_world_model->obs_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)potential_world_model->obs_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[potential_world_model->obs_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), train_obs[sample_index][step_index].begin(),
						train_obs[sample_index][step_index].end());
					potential_world_model->obs_networks[n_index]->activate(inputs);
					for (int o_index = 0; o_index < (int)potential_world_model->obs_network_outputs[n_index].size(); o_index++) {
						state[potential_world_model->obs_network_outputs[n_index][o_index]]
							+= potential_world_model->obs_networks[n_index]->output->acti_vals[o_index];
					}
				}

				vector<double> inputs;
				inputs.insert(inputs.end(), new_state.begin(), new_state.end());
				for (int i_index = 0; i_index < (int)new_obs_existing_inputs.size(); i_index++) {
					inputs.push_back(starting_state[new_obs_existing_inputs[i_index]]);
				}
				inputs.insert(inputs.end(), train_obs[sample_index][step_index].begin(),
					train_obs[sample_index][step_index].end());
				NetworkHistory* network_history = new NetworkHistory();
				new_obs_network->activate(inputs,
										  network_history);
				new_obs_network_histories.push_back(network_history);
				for (int o_index = 0; o_index < (int)new_state.size(); o_index++) {
					new_state[o_index] += new_obs_network->output->acti_vals[o_index];
				}
			}

			if (step_index < (int)train_actions[sample_index].size()) {
				int action = train_actions[sample_index][step_index];

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

				vector<double> inputs;
				inputs.insert(inputs.end(), new_state.begin(), new_state.end());
				for (int i_index = 0; i_index < (int)new_action_existing_inputs.size(); i_index++) {
					inputs.push_back(starting_state[new_action_existing_inputs[i_index]]);
				}
				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
				NetworkHistory* network_history = new NetworkHistory();
				new_action_network->activate(inputs,
											 network_history);
				new_action_network_histories.push_back(network_history);
				for (int o_index = 0; o_index < (int)new_state.size(); o_index++) {
					new_state[o_index] += new_action_network->output->acti_vals[o_index];
				}
			}
		}

		temp_final_network->activate(new_state);
		double predicted = temp_final_network->output->acti_vals[0];

		vector<double> score_errors{train_target_vals[sample_index] - predicted};
		
		// temp
		sum_error += abs(train_target_vals[sample_index] - predicted);

		vector<double> new_state_errors(NUM_NEW_STATES);

		temp_final_network->backprop(score_errors);
		for (int i_index = 0; i_index < NUM_NEW_STATES; i_index++) {
			new_state_errors[i_index] += temp_final_network->input->errors[i_index];
			temp_final_network->input->errors[i_index] = 0.0;
		}

		for (int step_index = (int)train_obs[sample_index].size()-1; step_index >= 0; step_index--) {
			if (step_index < (int)train_actions[sample_index].size()) {
				new_action_network->backprop(new_state_errors,
											 new_action_network_histories[step_index]);
				delete new_action_network_histories[step_index];
				for (int i_index = 0; i_index < NUM_NEW_STATES; i_index++) {
					new_state_errors[i_index] += new_action_network->input->errors[i_index];
					new_action_network->input->errors[i_index] = 0.0;
				}
			}

			if (step_index <= include_obs_index) {
				new_obs_network->backprop(new_state_errors,
										  new_obs_network_histories[step_index]);
				delete new_obs_network_histories[step_index];
				for (int i_index = 0; i_index < NUM_NEW_STATES; i_index++) {
					new_state_errors[i_index] += new_obs_network->input->errors[i_index];
					new_obs_network->input->errors[i_index] = 0.0;
				}
			}
		}

		if ((iter_index + 1)%NETWORK_EPOCH_SIZE == 0) {
			new_obs_network->update();
			new_action_network->update();
			temp_final_network->update();
		}

		if (iter_index % 1000 == 0) {
			cout << iter_index << ": " << sum_error << endl;
			sum_error = 0.0;
		}
	}

	vector<int> new_obs_network_inputs;
	for (int i_index = 0; i_index < NUM_NEW_STATES; i_index++) {
		new_obs_network_inputs.push_back(potential_world_model->num_states + i_index);
	}
	new_obs_network_inputs.insert(new_obs_network_inputs.end(),
		new_obs_existing_inputs.begin(), new_obs_existing_inputs.end());

	vector<int> new_action_network_inputs;
	for (int i_index = 0; i_index < NUM_NEW_STATES; i_index++) {
		new_action_network_inputs.push_back(potential_world_model->num_states + i_index);
	}
	new_action_network_inputs.insert(new_action_network_inputs.end(),
		new_action_existing_inputs.begin(), new_action_existing_inputs.end());

	vector<int> new_network_outputs;
	for (int i_index = 0; i_index < NUM_NEW_STATES; i_index++) {
		new_network_outputs.push_back(potential_world_model->num_states + i_index);
	}

	potential_world_model->num_states += NUM_NEW_STATES;

	for (int i_index = 0; i_index < (int)new_obs_network->input->errors.size(); i_index++) {
		new_obs_network->input->errors[i_index] = 0.0;
	}
	potential_world_model->obs_network_inputs.push_back(new_obs_network_inputs);
	potential_world_model->obs_network_outputs.push_back(new_network_outputs);
	potential_world_model->obs_networks.push_back(new_obs_network);

	for (int i_index = 0; i_index < (int)new_action_network->input->errors.size(); i_index++) {
		new_action_network->input->errors[i_index] = 0.0;
	}
	potential_world_model->action_network_inputs.push_back(new_action_network_inputs);
	potential_world_model->action_network_outputs.push_back(new_network_outputs);
	potential_world_model->action_networks.push_back(new_action_network);

	potential_world_model->score_network->add_inputs(NUM_NEW_STATES);

	delete temp_final_network;
}
