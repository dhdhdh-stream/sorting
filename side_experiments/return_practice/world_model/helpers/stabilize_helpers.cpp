#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "world_model_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int STABILIZE_TRAIN_ITERS = 10;
#else
const int STABILIZE_TRAIN_ITERS = 100000;
#endif /* MDEBUG */

void stabilize_helper(vector<Network*>& temp_obs_networks,
					  vector<double>& temp_obs_network_means,
					  vector<double>& temp_obs_network_diffs,
					  vector<Network*>& temp_action_networks,
					  vector<double>& temp_action_network_means,
					  vector<double>& temp_action_network_diffs,
					  Network*& new_final_network,
					  WorldModelWrapper* wrapper) {
	// temp
	cout << "stabilize_helper" << endl;

	WorldModel* world_model = wrapper->world_model;

	new_final_network = new Network(world_model->num_states + temp_obs_networks.size() + temp_action_networks.size(), 1);

	uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);
	for (int iter_index = 0; iter_index < STABILIZE_TRAIN_ITERS; iter_index++) {
		int sample_index = sample_distribution(generator);

		uniform_int_distribution<int> include_obs_distribution(
			0, wrapper->sample_obs[sample_index].size()-1);
		int include_obs_index = include_obs_distribution(generator);

		vector<double> state(world_model->num_states, 0.0);
		vector<double> new_state(temp_obs_networks.size() + temp_action_networks.size(), 0.0);

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

				vector<double> temp_inputs;
				temp_inputs.insert(temp_inputs.end(), starting_state.begin(), starting_state.end());
				temp_inputs.insert(temp_inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
					wrapper->sample_obs[sample_index][step_index].end());
				for (int n_index = 0; n_index < (int)temp_obs_networks.size(); n_index++) {
					temp_obs_networks[n_index]->activate(temp_inputs);
					double normalized = (temp_obs_networks[n_index]->output->acti_vals[0]
						- temp_obs_network_means[n_index]) / temp_obs_network_diffs[n_index];
					new_state[n_index] += normalized;
				}
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

				vector<double> temp_inputs;
				temp_inputs.insert(temp_inputs.end(), starting_state.begin(), starting_state.end());
				temp_inputs.insert(temp_inputs.end(), partial_inputs.begin(), partial_inputs.end());
				for (int n_index = 0; n_index < (int)temp_action_networks.size(); n_index++) {
					temp_action_networks[n_index]->activate(temp_inputs);
					double normalized = (temp_action_networks[n_index]->output->acti_vals[0]
						- temp_action_network_means[n_index]) / temp_action_network_diffs[n_index];
					new_state[(int)temp_obs_networks.size() + n_index] += normalized;
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

		vector<double> new_final_inputs;
		new_final_inputs.insert(new_final_inputs.end(), state.begin(), state.end());
		new_final_inputs.insert(new_final_inputs.end(), new_state.begin(), new_state.end());
		new_final_network->activate(new_final_inputs);
		sum_score += new_final_network->output->acti_vals[0];

		vector<double> final_errors{wrapper->sample_target_vals[sample_index] - sum_score};
		new_final_network->backprop(final_errors);

		if ((iter_index + 1)%NETWORK_EPOCH_SIZE == 0) {
			new_final_network->update();
		}
	}

	for (int i_index = 0; i_index < (int)new_final_network->input->errors.size(); i_index++) {
		new_final_network->input->errors[i_index] = 0.0;
	}

	// // temp
	// for (int iter_index = 0; iter_index < 20; iter_index++) {
	// 	cout << iter_index << endl;

	// 	int sample_index = sample_distribution(generator);

	// 	uniform_int_distribution<int> include_obs_distribution(
	// 		0, wrapper->sample_obs[sample_index].size()-1);
	// 	int include_obs_index = include_obs_distribution(generator);

	// 	vector<double> state(world_model->num_states, 0.0);
	// 	vector<double> new_state(temp_obs_networks.size() + temp_action_networks.size(), 0.0);

	// 	for (int step_index = 0; step_index < (int)wrapper->sample_obs[sample_index].size(); step_index++) {
	// 		if (step_index <= include_obs_index) {
	// 			vector<double> starting_state = state;

	// 			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
	// 				vector<double> inputs;
	// 				for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
	// 					inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
	// 				}
	// 				inputs.insert(inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
	// 					wrapper->sample_obs[sample_index][step_index].end());
	// 				world_model->obs_networks[n_index]->activate(inputs);
	// 				for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
	// 					state[world_model->obs_network_outputs[n_index][o_index]]
	// 						+= world_model->obs_networks[n_index]->output->acti_vals[o_index];
	// 				}
	// 			}

	// 			vector<double> temp_inputs;
	// 			temp_inputs.insert(temp_inputs.end(), starting_state.begin(), starting_state.end());
	// 			temp_inputs.insert(temp_inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
	// 				wrapper->sample_obs[sample_index][step_index].end());
	// 			for (int n_index = 0; n_index < (int)temp_obs_networks.size(); n_index++) {
	// 				temp_obs_networks[n_index]->activate(temp_inputs);
	// 				double normalized = (temp_obs_networks[n_index]->output->acti_vals[0]
	// 					- temp_obs_network_means[n_index]) / temp_obs_network_diffs[n_index];
	// 				new_state[n_index] += normalized;
	// 			}
	// 		}

	// 		if (step_index < (int)wrapper->sample_actions[sample_index].size()) {
	// 			int action = wrapper->sample_actions[sample_index][step_index];

	// 			vector<double> starting_state = state;

	// 			vector<double> partial_inputs;
	// 			for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
	// 				if (action == a_index) {
	// 					partial_inputs.push_back(1.0);
	// 				} else {
	// 					partial_inputs.push_back(0.0);
	// 				}
	// 			}

	// 			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
	// 				vector<double> inputs;
	// 				for (int i_index = 0; i_index < (int)world_model->action_network_inputs[n_index].size(); i_index++) {
	// 					inputs.push_back(starting_state[world_model->action_network_inputs[n_index][i_index]]);
	// 				}
	// 				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
	// 				world_model->action_networks[n_index]->activate(inputs);
	// 				for (int o_index = 0; o_index < (int)world_model->action_network_outputs[n_index].size(); o_index++) {
	// 					state[world_model->action_network_outputs[n_index][o_index]]
	// 						+= world_model->action_networks[n_index]->output->acti_vals[o_index];
	// 				}
	// 			}

	// 			vector<double> temp_inputs;
	// 			temp_inputs.insert(temp_inputs.end(), starting_state.begin(), starting_state.end());
	// 			temp_inputs.insert(temp_inputs.end(), partial_inputs.begin(), partial_inputs.end());
	// 			for (int n_index = 0; n_index < (int)temp_action_networks.size(); n_index++) {
	// 				temp_action_networks[n_index]->activate(temp_inputs);
	// 				double normalized = (temp_action_networks[n_index]->output->acti_vals[0]
	// 					- temp_action_network_means[n_index]) / temp_action_network_diffs[n_index];
	// 				new_state[(int)temp_obs_networks.size() + n_index] += normalized;
	// 			}
	// 		}
	// 	}

	// 	double sum_score = 0.0;
	// 	for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
	// 		vector<double> inputs;
	// 		for (int i_index = 0; i_index < (int)world_model->final_network_inputs[n_index].size(); i_index++) {
	// 			inputs.push_back(state[world_model->final_network_inputs[n_index][i_index]]);
	// 		}
	// 		world_model->final_networks[n_index]->activate(inputs);
	// 		sum_score += world_model->final_networks[n_index]->output->acti_vals[0];
	// 	}

	// 	vector<double> new_final_inputs;
	// 	new_final_inputs.insert(new_final_inputs.end(), state.begin(), state.end());
	// 	new_final_inputs.insert(new_final_inputs.end(), new_state.begin(), new_state.end());
	// 	new_final_network->activate(new_final_inputs);
	// 	sum_score += new_final_network->output->acti_vals[0];

	// 	cout << "sum_score: " << sum_score << endl;
	// 	cout << "wrapper->sample_target_vals[sample_index]: " << wrapper->sample_target_vals[sample_index] << endl;
	// }
}

void stabilize_action_final_helper(Network* temp_action_network,
								   double temp_action_network_mean,
								   double temp_action_network_diff,
								   Network*& new_final_network,
								   WorldModelWrapper* wrapper) {
	// temp
	cout << "stabilize_helper" << endl;

	WorldModel* world_model = wrapper->world_model;

	new_final_network = new Network(world_model->num_states + 1, 1);

	uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);
	for (int iter_index = 0; iter_index < STABILIZE_TRAIN_ITERS; iter_index++) {
		int sample_index = sample_distribution(generator);

		uniform_int_distribution<int> include_obs_distribution(
			0, wrapper->sample_obs[sample_index].size()-1);
		int include_obs_index = include_obs_distribution(generator);

		vector<double> state(world_model->num_states, 0.0);
		double new_state = 0.0;

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

				vector<double> temp_inputs;
				temp_inputs.insert(temp_inputs.end(), starting_state.begin(), starting_state.end());
				temp_inputs.insert(temp_inputs.end(), partial_inputs.begin(), partial_inputs.end());
				temp_action_network->activate(temp_inputs);
				double normalized = (temp_action_network->output->acti_vals[0]
					- temp_action_network_mean) / temp_action_network_diff;
				new_state += normalized;
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

		vector<double> new_final_inputs;
		new_final_inputs.insert(new_final_inputs.end(), state.begin(), state.end());
		new_final_inputs.push_back(new_state);
		new_final_network->activate(new_final_inputs);
		sum_score += new_final_network->output->acti_vals[0];

		vector<double> final_errors{wrapper->sample_target_vals[sample_index] - sum_score};
		new_final_network->backprop(final_errors);

		if ((iter_index + 1)%NETWORK_EPOCH_SIZE == 0) {
			new_final_network->update();
		}
	}

	for (int i_index = 0; i_index < (int)new_final_network->input->errors.size(); i_index++) {
		new_final_network->input->errors[i_index] = 0.0;
	}

	// // temp
	// for (int iter_index = 0; iter_index < 20; iter_index++) {
	// 	cout << iter_index << endl;

	// 	int sample_index = sample_distribution(generator);

	// 	uniform_int_distribution<int> include_obs_distribution(
	// 		0, wrapper->sample_obs[sample_index].size()-1);
	// 	int include_obs_index = include_obs_distribution(generator);

	// 	vector<double> state(world_model->num_states, 0.0);
	// 	double new_state = 0.0;

	// 	for (int step_index = 0; step_index < (int)wrapper->sample_obs[sample_index].size(); step_index++) {
	// 		if (step_index <= include_obs_index) {
	// 			vector<double> starting_state = state;

	// 			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
	// 				vector<double> inputs;
	// 				for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
	// 					inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
	// 				}
	// 				inputs.insert(inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
	// 					wrapper->sample_obs[sample_index][step_index].end());
	// 				world_model->obs_networks[n_index]->activate(inputs);
	// 				for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
	// 					state[world_model->obs_network_outputs[n_index][o_index]]
	// 						+= world_model->obs_networks[n_index]->output->acti_vals[o_index];
	// 				}
	// 			}
	// 		}

	// 		if (step_index < (int)wrapper->sample_actions[sample_index].size()) {
	// 			int action = wrapper->sample_actions[sample_index][step_index];

	// 			vector<double> starting_state = state;

	// 			vector<double> partial_inputs;
	// 			for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
	// 				if (action == a_index) {
	// 					partial_inputs.push_back(1.0);
	// 				} else {
	// 					partial_inputs.push_back(0.0);
	// 				}
	// 			}

	// 			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
	// 				vector<double> inputs;
	// 				for (int i_index = 0; i_index < (int)world_model->action_network_inputs[n_index].size(); i_index++) {
	// 					inputs.push_back(starting_state[world_model->action_network_inputs[n_index][i_index]]);
	// 				}
	// 				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
	// 				world_model->action_networks[n_index]->activate(inputs);
	// 				for (int o_index = 0; o_index < (int)world_model->action_network_outputs[n_index].size(); o_index++) {
	// 					state[world_model->action_network_outputs[n_index][o_index]]
	// 						+= world_model->action_networks[n_index]->output->acti_vals[o_index];
	// 				}
	// 			}

	// 			vector<double> temp_inputs;
	// 			temp_inputs.insert(temp_inputs.end(), starting_state.begin(), starting_state.end());
	// 			temp_inputs.insert(temp_inputs.end(), partial_inputs.begin(), partial_inputs.end());
	// 			temp_action_network->activate(temp_inputs);
	// 			double normalized = (temp_action_network->output->acti_vals[0]
	// 				- temp_action_network_mean) / temp_action_network_diff;
	// 			new_state += normalized;
	// 		}
	// 	}

	// 	double sum_score = 0.0;
	// 	for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
	// 		vector<double> inputs;
	// 		for (int i_index = 0; i_index < (int)world_model->final_network_inputs[n_index].size(); i_index++) {
	// 			inputs.push_back(state[world_model->final_network_inputs[n_index][i_index]]);
	// 		}
	// 		world_model->final_networks[n_index]->activate(inputs);
	// 		sum_score += world_model->final_networks[n_index]->output->acti_vals[0];
	// 	}

	// 	vector<double> new_final_inputs;
	// 	new_final_inputs.insert(new_final_inputs.end(), state.begin(), state.end());
	// 	new_final_inputs.push_back(new_state);
	// 	new_final_network->activate(new_final_inputs);
	// 	sum_score += new_final_network->output->acti_vals[0];

	// 	cout << "sum_score: " << sum_score << endl;
	// 	cout << "wrapper->sample_target_vals[sample_index]: " << wrapper->sample_target_vals[sample_index] << endl;
	// }
}

void stabilize_obs_state_helper(Network* temp_obs_network,
								double temp_obs_network_mean,
								double temp_obs_network_diff,
								int state_index,
								Network*& new_obs_existing_network,
								Network*& new_action_existing_network,
								WorldModelWrapper* wrapper) {
	// temp
	cout << "stabilize_helper" << endl;

	WorldModel* world_model = wrapper->world_model;

	vector<int> dependencies = world_model->state_dependencies[state_index];
	new_obs_existing_network = new Network(dependencies.size() + 1 + wrapper->num_obs, 1);
	new_action_existing_network = new Network(dependencies.size() + 1 + wrapper->num_actions, 1);

	uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);
	for (int iter_index = 0; iter_index < STABILIZE_TRAIN_ITERS; iter_index++) {
		int sample_index = sample_distribution(generator);

		uniform_int_distribution<int> include_obs_distribution(
			0, wrapper->sample_obs[sample_index].size()-1);
		int include_obs_index = include_obs_distribution(generator);

		vector<double> state(world_model->num_states, 0.0);
		double new_state = 0.0;

		vector<vector<NetworkHistory*>> obs_network_histories;
		vector<NetworkHistory*> new_existing_obs_network_histories;
		vector<vector<NetworkHistory*>> action_network_histories;
		vector<NetworkHistory*> new_existing_action_network_histories;

		for (int step_index = 0; step_index < (int)wrapper->sample_obs[sample_index].size(); step_index++) {
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

				vector<double> temp_inputs = wrapper->sample_obs[sample_index][step_index];
				temp_obs_network->activate(temp_inputs);
				double normalized = (temp_obs_network->output->acti_vals[0]
					- temp_obs_network_mean) / temp_obs_network_diff;
				new_state += normalized;

				vector<double> new_existing_inputs;
				for (int i_index = 0; i_index < (int)dependencies.size(); i_index++) {
					new_existing_inputs.push_back(starting_state[dependencies[i_index]]);
				}
				new_existing_inputs.push_back(new_state);
				new_existing_inputs.insert(new_existing_inputs.end(), wrapper->sample_obs[sample_index][step_index].begin(),
					wrapper->sample_obs[sample_index][step_index].end());
				NetworkHistory* new_existing_network_history = new NetworkHistory();
				new_obs_existing_network->activate(new_existing_inputs,
												   new_existing_network_history);
				new_existing_obs_network_histories.push_back(new_existing_network_history);
				state[state_index] += new_obs_existing_network->output->acti_vals[0];
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

				vector<double> new_existing_inputs;
				for (int i_index = 0; i_index < (int)dependencies.size(); i_index++) {
					new_existing_inputs.push_back(starting_state[dependencies[i_index]]);
				}
				new_existing_inputs.push_back(new_state);
				new_existing_inputs.insert(new_existing_inputs.end(), partial_inputs.begin(), partial_inputs.end());
				NetworkHistory* new_existing_network_history = new NetworkHistory();
				new_action_existing_network->activate(new_existing_inputs,
													  new_existing_network_history);
				new_existing_action_network_histories.push_back(new_existing_network_history);
				state[state_index] += new_action_existing_network->output->acti_vals[0];
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

				vector<double> new_existing_errors{starting_errors[state_index]};
				new_action_existing_network->backprop(new_existing_errors,
													  new_existing_action_network_histories[step_index]);
				delete new_existing_action_network_histories[step_index];
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

				vector<double> new_existing_errors{starting_errors[state_index]};
				new_obs_existing_network->backprop(new_existing_errors,
												   new_existing_obs_network_histories[step_index]);
				delete new_existing_obs_network_histories[step_index];
			}
		}

		if ((iter_index + 1)%NETWORK_EPOCH_SIZE == 0) {
			new_obs_existing_network->update();
			new_action_existing_network->update();
		}
	}

	for (int i_index = 0; i_index < (int)new_obs_existing_network->input->errors.size(); i_index++) {
		new_obs_existing_network->input->errors[i_index] = 0.0;
	}
	for (int i_index = 0; i_index < (int)new_action_existing_network->input->errors.size(); i_index++) {
		new_action_existing_network->input->errors[i_index] = 0.0;
	}
}
