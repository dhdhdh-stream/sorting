#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "world_model_wrapper.h"

using namespace std;

const int NUM_EXISTING_STATES = 8;
const int NUM_NEW_STATES = 4;

#if defined(MDEBUG) && MDEBUG
const int UPDATE_ITERS = 30;
const int INIT_TRAIN_ITERS = 30;
const int TRAIN_ITERS = 30;
#else
const int UPDATE_ITERS = 100000;
const int INIT_TRAIN_ITERS = 150000;
const int TRAIN_ITERS = 150000;
#endif /* MDEBUG */

const double VERIFICATION_RATIO = 0.2;

const int TRAIN_TRY_ITERS = 10;

void train_iter_helper(vector<vector<double>>& sample_obs,
					   vector<int>& sample_actions,
					   double sample_target_val,
					   int include_obs_index,
					   WorldModel* world_model,
					   WorldModelWrapper* wrapper) {
	vector<double> state(world_model->num_states, 0.0);

	vector<vector<NetworkHistory*>> obs_network_histories;
	vector<vector<NetworkHistory*>> action_network_histories;

	for (int step_index = 0; step_index < (int)sample_obs.size(); step_index++) {
		if (step_index <= include_obs_index) {
			vector<double> starting_state = state;

			vector<NetworkHistory*> step_obs_network_histories;
			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
					inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), sample_obs[step_index].begin(),
					sample_obs[step_index].end());
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

		if (step_index < (int)sample_actions.size()) {
			int action = sample_actions[step_index];

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

	vector<double> final_errors{sample_target_val - sum_score};

	vector<double> state_errors(world_model->num_states, 0.0);

	for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
		world_model->final_networks[n_index]->backprop(final_errors);
		for (int i_index = 0; i_index < (int)world_model->final_network_inputs[n_index].size(); i_index++) {
			state_errors[world_model->final_network_inputs[n_index][i_index]]
				+= world_model->final_networks[n_index]->input->errors[i_index];
			world_model->final_networks[n_index]->input->errors[i_index] = 0.0;
		}
	}

	for (int step_index = (int)sample_obs.size()-1; step_index >= 0; step_index--) {
		if (step_index < (int)sample_actions.size()) {
			vector<double> starting_errors = state_errors;

			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
				vector<double> errors;
				for (int o_index = 0; o_index < (int)world_model->action_network_outputs[n_index].size(); o_index++) {
					errors.push_back(starting_errors[world_model->action_network_outputs[n_index][o_index]]);
				}
				world_model->action_networks[n_index]->backprop(errors,
																action_network_histories[step_index][n_index]);
				delete action_network_histories[step_index][n_index];
				for (int i_index = 0; i_index < (int)world_model->action_network_inputs[n_index].size(); i_index++) {
					state_errors[world_model->action_network_inputs[n_index][i_index]]
						+= world_model->action_networks[n_index]->input->errors[i_index];
					world_model->action_networks[n_index]->input->errors[i_index] = 0.0;
				}
			}
		}

		if (step_index <= include_obs_index) {
			vector<double> starting_errors = state_errors;

			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> errors;
				for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
					errors.push_back(starting_errors[world_model->obs_network_outputs[n_index][o_index]]);
				}
				world_model->obs_networks[n_index]->backprop(errors,
															 obs_network_histories[step_index][n_index]);
				delete obs_network_histories[step_index][n_index];
				for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
					state_errors[world_model->obs_network_inputs[n_index][i_index]]
						+= world_model->obs_networks[n_index]->input->errors[i_index];
					world_model->obs_networks[n_index]->input->errors[i_index] = 0.0;
				}
			}
		}
	}
}

void update_helper(WorldModel* world_model) {
	double max_update = 0.0;
	for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
		world_model->obs_networks[n_index]->get_max_update(max_update);
	}
	for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
		world_model->action_networks[n_index]->get_max_update(max_update);
	}
	for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
		world_model->final_networks[n_index]->get_max_update(max_update);
	}

	world_model->average_max_update += 0.999*world_model->average_max_update + 0.001*max_update;

	if (max_update > 0.0) {
		double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / world_model->average_max_update;
		if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
			learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
		}

		for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
			world_model->obs_networks[n_index]->update_weights(learning_rate);
		}
		for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
			world_model->action_networks[n_index]->update_weights(learning_rate);
		}
		for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
			world_model->final_networks[n_index]->update_weights(learning_rate);
		}
	}
}

void init_train_helper(vector<vector<vector<double>>>& train_obs,
					   vector<vector<int>>& train_actions,
					   vector<double>& train_target_vals,
					   WorldModel* potential_world_model,
					   WorldModelWrapper* wrapper) {
	// temp
	cout << "init_train_helper" << endl;

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

	vector<int> new_final_existing_inputs;
	{
		vector<int> remaining_indexes;
		for (int s_index = 0; s_index < potential_world_model->num_states; s_index++) {
			remaining_indexes.push_back(s_index);
		}

		while (remaining_indexes.size() > 0) {
			uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
			int index = distribution(generator);
			new_final_existing_inputs.push_back(index);
			remaining_indexes.erase(remaining_indexes.begin() + index);

			if (new_final_existing_inputs.size() >= NUM_EXISTING_STATES) {
				break;
			}
		}
	}
	Network* new_final_network = new Network(NUM_NEW_STATES + new_final_existing_inputs.size(), 1);

	uniform_int_distribution<int> sample_distribution(0, train_obs.size()-1);
	for (int iter_index = 0; iter_index < INIT_TRAIN_ITERS; iter_index++) {
		int sample_index = sample_distribution(generator);

		uniform_int_distribution<int> include_obs_distribution(
			0, train_obs[sample_index].size()-1);
		int include_obs_index = include_obs_distribution(generator);

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
		new_final_inputs.insert(new_final_inputs.end(), new_state.begin(), new_state.end());
		for (int i_index = 0; i_index < (int)new_final_existing_inputs.size(); i_index++) {
			new_final_inputs.push_back(state[new_final_existing_inputs[i_index]]);
		}
		new_final_network->activate(new_final_inputs);
		sum_score += new_final_network->output->acti_vals[0];

		vector<double> final_errors{train_target_vals[sample_index] - sum_score};

		vector<double> new_state_errors(NUM_NEW_STATES, 0.0);

		new_final_network->backprop(final_errors);
		for (int i_index = 0; i_index < NUM_NEW_STATES; i_index++) {
			new_state_errors[i_index] += new_final_network->input->errors[i_index];
			new_final_network->input->errors[i_index] = 0.0;
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
			new_final_network->update();
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

	vector<int> new_final_network_inputs;
	for (int i_index = 0; i_index < NUM_NEW_STATES; i_index++) {
		new_final_network_inputs.push_back(potential_world_model->num_states + i_index);
	}
	new_final_network_inputs.insert(new_final_network_inputs.end(),
		new_final_existing_inputs.begin(), new_final_existing_inputs.end());

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

	for (int i_index = 0; i_index < (int)new_final_network->input->errors.size(); i_index++) {
		new_final_network->input->errors[i_index] = 0.0;
	}
	potential_world_model->final_network_inputs.push_back(new_final_network_inputs);
	potential_world_model->final_networks.push_back(new_final_network);
}

void train_helper(WorldModelWrapper* wrapper) {
	// temp
	cout << "train_helper" << endl;

	for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
		vector<vector<double>> sample_obs;
		vector<int> sample_actions;
		double sample_target_val;
		if (wrapper->new_sample_obs.size() >= wrapper->old_sample_obs.size()) {
			uniform_int_distribution<int> is_old_distribution(
				0, wrapper->old_sample_obs.size() + wrapper->new_sample_obs.size() - 1);
			if (is_old_distribution(generator) < (int)wrapper->old_sample_obs.size()) {
				uniform_int_distribution<int> old_distribution(0, wrapper->old_sample_obs.size()-1);
				int index = old_distribution(generator);
				sample_obs = wrapper->old_sample_obs[index];
				sample_actions = wrapper->old_sample_actions[index];
				sample_target_val = wrapper->old_sample_target_vals[index];
			} else {
				uniform_int_distribution<int> new_distribution(0, wrapper->new_sample_obs.size()-1);
				int index = new_distribution(generator);
				sample_obs = wrapper->new_sample_obs[index];
				sample_actions = wrapper->new_sample_actions[index];
				sample_target_val = wrapper->new_sample_target_vals[index];
			}
		} else {
			uniform_int_distribution<int> is_old_distribution(0, 1);
			if (is_old_distribution(generator) == 0) {
				uniform_int_distribution<int> old_distribution(0, wrapper->old_sample_obs.size()-1);
				int index = old_distribution(generator);
				sample_obs = wrapper->old_sample_obs[index];
				sample_actions = wrapper->old_sample_actions[index];
				sample_target_val = wrapper->old_sample_target_vals[index];
			} else {
				uniform_int_distribution<int> new_distribution(0, wrapper->new_sample_obs.size()-1);
				int index = new_distribution(generator);
				sample_obs = wrapper->new_sample_obs[index];
				sample_actions = wrapper->new_sample_actions[index];
				sample_target_val = wrapper->new_sample_target_vals[index];
			}
		}

		uniform_int_distribution<int> include_obs_distribution(
			0, sample_obs.size()-1);
		int include_obs_index = include_obs_distribution(generator);

		train_iter_helper(sample_obs,
						  sample_actions,
						  sample_target_val,
						  include_obs_index,
						  wrapper->world_model,
						  wrapper);

		if ((iter_index + 1)%NETWORK_EPOCH_SIZE == 0) {
			update_helper(wrapper->world_model);
		}
	}

	for (int try_index = 0; try_index < TRAIN_TRY_ITERS; try_index++) {
		vector<vector<vector<double>>> train_obs;
		vector<vector<int>> train_actions;
		vector<double> train_target_vals;
		vector<vector<vector<double>>> verification_obs;
		vector<vector<int>> verification_actions;
		vector<double> verification_target_vals;

		int num_new_verification = VERIFICATION_RATIO * (double)wrapper->new_sample_obs.size();
		int num_new_train = (int)wrapper->new_sample_obs.size() - num_new_verification;



		int num_old_verification;
		int num_old_train;
		if (wrapper->new_sample_obs.size() >= wrapper->old_sample_obs.size()) {

		} else {

		}

		WorldModel* potential_world_model = new WorldModel(wrapper->world_model);



		delete wrapper->world_model;
		wrapper->world_model = potential_world_model;
	}

	// temp
	cout << "train_helper done" << endl;
}
