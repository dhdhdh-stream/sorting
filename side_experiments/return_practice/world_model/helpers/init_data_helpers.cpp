#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "world_model_wrapper.h"

using namespace std;

const int INIT_FACTOR_NUM_DATA = 1000;

void init_data_helper(vector<vector<double>>& init_obs_inputs,
					  vector<vector<double>>& init_action_inputs,
					  vector<double>& init_target_vals,
					  WorldModelWrapper* wrapper) {
	// temp
	cout << "init_data_helper" << endl;

	WorldModel* world_model = wrapper->world_model;

	uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);
	for (int gather_index = 0; gather_index < INIT_FACTOR_NUM_DATA; gather_index++) {
		int sample_index = sample_distribution(generator);

		uniform_int_distribution<int> obs_select_distribution(
			0, wrapper->sample_obs[sample_index].size()-1);
		int obs_select_index = obs_select_distribution(generator);

		uniform_int_distribution<int> action_select_distribution(
			0, wrapper->sample_actions[sample_index].size()-1);
		int action_select_index = action_select_distribution(generator);

		uniform_int_distribution<int> include_obs_distribution(
			obs_select_index, wrapper->sample_obs[sample_index].size()-1);
		int include_obs_index = include_obs_distribution(generator);

		vector<double> state(world_model->num_states, 0.0);

		for (int step_index = 0; step_index < (int)wrapper->sample_obs[sample_index].size(); step_index++) {
			if (step_index == obs_select_index) {
				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				inputs.insert(inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
					wrapper->sample_obs[sample_index][step_index].end());
				init_obs_inputs.push_back(inputs);
			}

			if (step_index <= include_obs_index) {
				vector<double> starting_state = state;

				for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
						wrapper->sample_obs[sample_index][step_index].end());
					world_model->obs_networks[n_index]->activate(inputs);
					for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
						state[world_model->obs_network_outputs[n_index][o_index]]
							+= world_model->obs_networks[n_index]->output->acti_vals[o_index];
					}
				}
			}

			if (step_index == action_select_index) {
				int action = wrapper->sample_actions[sample_index][step_index];

				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				init_action_inputs.push_back(inputs);
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

				for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)world_model->action_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[world_model->action_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
					world_model->action_networks[n_index]->activate(inputs);
					for (int o_index = 0; o_index < (int)world_model->action_network_outputs[n_index].size(); o_index++) {
						state[world_model->action_network_outputs[n_index][o_index]]
							+= world_model->action_networks[n_index]->output->acti_vals[o_index];
					}
				}
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

		init_target_vals.push_back((wrapper->sample_target_vals[sample_index] - sum_score)
			/ (double)wrapper->sample_obs[sample_index].size());
	}
}

void init_action_final_data_helper(vector<vector<double>>& init_action_inputs,
								   vector<double>& init_target_vals,
								   WorldModelWrapper* wrapper) {
	// temp
	cout << "init_data_helper" << endl;

	WorldModel* world_model = wrapper->world_model;

	uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);
	for (int gather_index = 0; gather_index < INIT_FACTOR_NUM_DATA; gather_index++) {
		int sample_index = sample_distribution(generator);

		uniform_int_distribution<int> action_select_distribution(
			0, wrapper->sample_actions[sample_index].size()-1);
		int action_select_index = action_select_distribution(generator);

		uniform_int_distribution<int> include_obs_distribution(
			0, wrapper->sample_obs[sample_index].size()-1);
		int include_obs_index = include_obs_distribution(generator);

		vector<double> state(world_model->num_states, 0.0);

		for (int step_index = 0; step_index < (int)wrapper->sample_obs[sample_index].size(); step_index++) {
			if (step_index <= include_obs_index) {
				vector<double> starting_state = state;

				for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
						wrapper->sample_obs[sample_index][step_index].end());
					world_model->obs_networks[n_index]->activate(inputs);
					for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
						state[world_model->obs_network_outputs[n_index][o_index]]
							+= world_model->obs_networks[n_index]->output->acti_vals[o_index];
					}
				}
			}

			if (step_index == action_select_index) {
				int action = wrapper->sample_actions[sample_index][step_index];

				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				init_action_inputs.push_back(inputs);
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

				for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)world_model->action_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[world_model->action_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
					world_model->action_networks[n_index]->activate(inputs);
					for (int o_index = 0; o_index < (int)world_model->action_network_outputs[n_index].size(); o_index++) {
						state[world_model->action_network_outputs[n_index][o_index]]
							+= world_model->action_networks[n_index]->output->acti_vals[o_index];
					}
				}
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

		init_target_vals.push_back((wrapper->sample_target_vals[sample_index] - sum_score)
			/ (double)wrapper->sample_obs[sample_index].size());
	}
}

void init_obs_state_data_helper(int state_index,
								vector<vector<double>>& init_obs_inputs,
								vector<double>& init_target_vals,
								WorldModelWrapper* wrapper) {
	// temp
	cout << "init_data_helper" << endl;

	WorldModel* world_model = wrapper->world_model;

	uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);
	for (int gather_index = 0; gather_index < INIT_FACTOR_NUM_DATA; gather_index++) {
		int sample_index = sample_distribution(generator);

		uniform_int_distribution<int> obs_select_distribution(
			0, wrapper->sample_obs[sample_index].size()-1);
		int obs_select_index = obs_select_distribution(generator);

		uniform_int_distribution<int> include_obs_distribution(
			obs_select_index, wrapper->sample_obs[sample_index].size()-1);
		int include_obs_index = include_obs_distribution(generator);

		vector<double> state(world_model->num_states, 0.0);

		vector<vector<NetworkHistory*>> obs_network_histories;
		vector<vector<NetworkHistory*>> action_network_histories;

		for (int step_index = 0; step_index < (int)wrapper->sample_obs[sample_index].size(); step_index++) {
			if (step_index == obs_select_index) {
				vector<double> inputs;
				inputs.insert(inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
					wrapper->sample_obs[sample_index][step_index].end());
				init_obs_inputs.push_back(inputs);
			}

			if (step_index <= include_obs_index) {
				vector<double> starting_state = state;

				vector<NetworkHistory*> step_obs_network_histories;
				for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
						inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
					}
					inputs.insert(inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
						wrapper->sample_obs[sample_index][step_index].end());
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

		vector<double> final_errors{wrapper->sample_target_vals[sample_index] - sum_score};

		vector<double> state_errors(world_model->num_states, 0.0);

		for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
			world_model->final_networks[n_index]->backprop_through(final_errors);
			for (int i_index = 0; i_index < (int)world_model->final_network_inputs[n_index].size(); i_index++) {
				state_errors[world_model->final_network_inputs[n_index][i_index]]
					+= world_model->final_networks[n_index]->input->errors[i_index];
				world_model->final_networks[n_index]->input->errors[i_index] = 0.0;
			}
		}

		for (int step_index = (int)wrapper->sample_obs[sample_index].size()-1; step_index >= 0; step_index--) {
			if (step_index < (int)wrapper->sample_actions[sample_index].size()) {
				vector<double> starting_errors = state_errors;

				for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
					vector<double> errors;
					for (int o_index = 0; o_index < (int)world_model->action_network_outputs[n_index].size(); o_index++) {
						errors.push_back(starting_errors[world_model->action_network_outputs[n_index][o_index]]);
					}
					world_model->action_networks[n_index]->backprop_through(errors,
																			action_network_histories[step_index][n_index]);
					delete action_network_histories[step_index][n_index];
					for (int i_index = 0; i_index < (int)world_model->action_network_inputs[n_index].size(); i_index++) {
						state_errors[world_model->action_network_inputs[n_index][i_index]]
							+= world_model->action_networks[n_index]->input->errors[i_index];
						world_model->action_networks[n_index]->input->errors[i_index] = 0.0;
					}
				}
			}

			if (step_index == obs_select_index) {
				init_target_vals.push_back(state_errors[state_index]);
			}

			if (step_index <= include_obs_index) {
				vector<double> starting_errors = state_errors;

				for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
					vector<double> errors;
					for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
						errors.push_back(starting_errors[world_model->obs_network_outputs[n_index][o_index]]);
					}
					world_model->obs_networks[n_index]->backprop_through(errors,
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
}
