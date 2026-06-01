// - not that effective
//   - predicted misguess is large where weak
//     - so doesn't focus on weak

#include "world_model_helpers.h"

#include <algorithm>
#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

const int NUM_EXISTING_STATES = 8;
const int NUM_NEW_STATES = 2;

#if defined(MDEBUG) && MDEBUG
const int UPDATE_ITERS = 20;
const int TRAIN_ITERS = 30;
const int EVAL_ITERS = 10;
#else
const int UPDATE_ITERS = 100000;
const int TRAIN_ITERS = 300000;
// const int EVAL_ITERS = 4000;
const int EVAL_ITERS = 8000;
#endif /* MDEBUG */

const double VERIFICATION_RATIO = 0.2;

void train_helper(vector<vector<vector<double>>>& train_obs,
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
	Network* new_score_network = new Network(NUM_NEW_STATES + new_final_existing_inputs.size(), 1);
	Network* new_misguess_network = new Network(NUM_NEW_STATES + new_final_existing_inputs.size(), 1);

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

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)potential_world_model->score_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)potential_world_model->score_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[potential_world_model->score_network_inputs[n_index][i_index]]);
			}
			potential_world_model->score_networks[n_index]->activate(inputs);
			sum_score += potential_world_model->score_networks[n_index]->output->acti_vals[0];
		}

		double sum_misguess = 0.0;
		for (int n_index = 0; n_index < (int)potential_world_model->misguess_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)potential_world_model->misguess_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[potential_world_model->misguess_network_inputs[n_index][i_index]]);
			}
			potential_world_model->misguess_networks[n_index]->activate(inputs);
			sum_misguess += potential_world_model->misguess_networks[n_index]->output->acti_vals[0];
		}

		vector<double> new_final_inputs;
		new_final_inputs.insert(new_final_inputs.end(), new_state.begin(), new_state.end());
		for (int i_index = 0; i_index < (int)new_final_existing_inputs.size(); i_index++) {
			new_final_inputs.push_back(state[new_final_existing_inputs[i_index]]);
		}
		new_score_network->activate(new_final_inputs);
		sum_score += new_score_network->output->acti_vals[0];
		new_misguess_network->activate(new_final_inputs);
		sum_misguess += new_misguess_network->output->acti_vals[0];

		vector<double> score_errors{train_target_vals[sample_index] - sum_score};
		double misguess = abs(train_target_vals[sample_index] - sum_score);
		// temp
		sum_error += misguess;
		vector<double> misguess_errors{misguess - sum_misguess};

		vector<double> new_state_errors(NUM_NEW_STATES, 0.0);

		new_score_network->backprop(score_errors);
		for (int i_index = 0; i_index < NUM_NEW_STATES; i_index++) {
			new_state_errors[i_index] += new_score_network->input->errors[i_index];
			new_score_network->input->errors[i_index] = 0.0;
		}

		new_misguess_network->backprop(misguess_errors);
		for (int i_index = 0; i_index < NUM_NEW_STATES; i_index++) {
			new_state_errors[i_index] += new_misguess_network->input->errors[i_index];
			new_misguess_network->input->errors[i_index] = 0.0;
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
			new_score_network->update();
			new_misguess_network->update();
		}

		if (iter_index % 1000 == 0) {
			cout << iter_index << ": " << sum_error << endl;
			sum_error = 0.0;
		}
	}

	// // temp
	// for (int h_index = 0; h_index < 5; h_index++) {
	// 	cout << "h_index: " << h_index << endl;

	// 	uniform_int_distribution<int> include_obs_distribution(
	// 		0, train_obs[h_index].size()-1);
	// 	// int include_obs_index = include_obs_distribution(generator);
	// 	int include_obs_index = train_obs[h_index].size()-1;

	// 	vector<double> state(potential_world_model->num_states, 0.0);
	// 	vector<double> new_state(NUM_NEW_STATES, 0.0);

	// 	for (int step_index = 0; step_index < (int)train_obs[h_index].size(); step_index++) {
	// 		if (step_index <= include_obs_index) {
	// 			vector<double> starting_state = state;

	// 			for (int n_index = 0; n_index < (int)potential_world_model->obs_networks.size(); n_index++) {
	// 				vector<double> inputs;
	// 				for (int i_index = 0; i_index < (int)potential_world_model->obs_network_inputs[n_index].size(); i_index++) {
	// 					inputs.push_back(starting_state[potential_world_model->obs_network_inputs[n_index][i_index]]);
	// 				}
	// 				inputs.insert(inputs.end(), train_obs[h_index][step_index].begin(),
	// 					train_obs[h_index][step_index].end());
	// 				potential_world_model->obs_networks[n_index]->activate(inputs);
	// 				for (int o_index = 0; o_index < (int)potential_world_model->obs_network_outputs[n_index].size(); o_index++) {
	// 					state[potential_world_model->obs_network_outputs[n_index][o_index]]
	// 						+= potential_world_model->obs_networks[n_index]->output->acti_vals[o_index];
	// 				}
	// 			}

	// 			vector<double> inputs;
	// 			inputs.insert(inputs.end(), new_state.begin(), new_state.end());
	// 			for (int i_index = 0; i_index < (int)new_obs_existing_inputs.size(); i_index++) {
	// 				inputs.push_back(starting_state[new_obs_existing_inputs[i_index]]);
	// 			}
	// 			inputs.insert(inputs.end(), train_obs[h_index][step_index].begin(),
	// 				train_obs[h_index][step_index].end());
	// 			new_obs_network->activate(inputs);
	// 			for (int o_index = 0; o_index < (int)new_state.size(); o_index++) {
	// 				new_state[o_index] += new_obs_network->output->acti_vals[o_index];
	// 			}
	// 		}

	// 		if (step_index < (int)train_actions[h_index].size()) {
	// 			int action = train_actions[h_index][step_index];

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

	// 			vector<double> inputs;
	// 			inputs.insert(inputs.end(), new_state.begin(), new_state.end());
	// 			for (int i_index = 0; i_index < (int)new_action_existing_inputs.size(); i_index++) {
	// 				inputs.push_back(starting_state[new_action_existing_inputs[i_index]]);
	// 			}
	// 			inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
	// 			new_action_network->activate(inputs);
	// 			for (int o_index = 0; o_index < (int)new_state.size(); o_index++) {
	// 				new_state[o_index] += new_action_network->output->acti_vals[o_index];
	// 			}
	// 		}
	// 	}

	// 	double sum_score = 0.0;
	// 	for (int n_index = 0; n_index < (int)potential_world_model->score_networks.size(); n_index++) {
	// 		vector<double> inputs;
	// 		for (int i_index = 0; i_index < (int)potential_world_model->score_network_inputs[n_index].size(); i_index++) {
	// 			inputs.push_back(state[potential_world_model->score_network_inputs[n_index][i_index]]);
	// 		}
	// 		potential_world_model->score_networks[n_index]->activate(inputs);
	// 		sum_score += potential_world_model->score_networks[n_index]->output->acti_vals[0];
	// 		cout << "potential_world_model->score_networks[n_index]->output->acti_vals[0]: " << potential_world_model->score_networks[n_index]->output->acti_vals[0] << endl;
	// 	}

	// 	double sum_misguess = 0.0;
	// 	for (int n_index = 0; n_index < (int)potential_world_model->misguess_networks.size(); n_index++) {
	// 		vector<double> inputs;
	// 		for (int i_index = 0; i_index < (int)potential_world_model->misguess_network_inputs[n_index].size(); i_index++) {
	// 			inputs.push_back(state[potential_world_model->misguess_network_inputs[n_index][i_index]]);
	// 		}
	// 		potential_world_model->misguess_networks[n_index]->activate(inputs);
	// 		sum_misguess += potential_world_model->misguess_networks[n_index]->output->acti_vals[0];
	// 	}

	// 	vector<double> new_final_inputs;
	// 	new_final_inputs.insert(new_final_inputs.end(), new_state.begin(), new_state.end());
	// 	for (int i_index = 0; i_index < (int)new_final_existing_inputs.size(); i_index++) {
	// 		new_final_inputs.push_back(state[new_final_existing_inputs[i_index]]);
	// 	}
	// 	// temp
	// 	cout << "new_final_inputs:";
	// 	for (int i_index = 0; i_index < (int)new_final_inputs.size(); i_index++) {
	// 		cout << " " << new_final_inputs[i_index];
	// 	}
	// 	cout << endl;
	// 	new_score_network->activate(new_final_inputs);
	// 	sum_score += new_score_network->output->acti_vals[0];
	// 	cout << "new_score_network->output->acti_vals[0]: " << new_score_network->output->acti_vals[0] << endl;
	// 	new_misguess_network->activate(new_final_inputs);
	// 	sum_misguess += new_misguess_network->output->acti_vals[0];

	// 	cout << "new_state:";
	// 	for (int s_index = 0; s_index < (int)new_state.size(); s_index++) {
	// 		cout << " " << new_state[s_index];
	// 	}
	// 	cout << endl;
	// 	cout << "state:";
	// 	for (int s_index = 0; s_index < (int)state.size(); s_index++) {
	// 		cout << " " << state[s_index];
	// 	}
	// 	cout << endl;
	// 	cout << "sum_score: " << sum_score << endl;
	// }

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

	for (int i_index = 0; i_index < (int)new_score_network->input->errors.size(); i_index++) {
		new_score_network->input->errors[i_index] = 0.0;
	}
	potential_world_model->score_network_inputs.push_back(new_final_network_inputs);
	potential_world_model->score_networks.push_back(new_score_network);

	for (int i_index = 0; i_index < (int)new_misguess_network->input->errors.size(); i_index++) {
		new_misguess_network->input->errors[i_index] = 0.0;
	}
	potential_world_model->misguess_network_inputs.push_back(new_final_network_inputs);
	potential_world_model->misguess_networks.push_back(new_misguess_network);

	// // temp
	// for (int h_index = 0; h_index < 5; h_index++) {
	// 	cout << "h_index: " << h_index << endl;

	// 	uniform_int_distribution<int> include_obs_distribution(
	// 		0, train_obs[h_index].size()-1);
	// 	// int include_obs_index = include_obs_distribution(generator);
	// 	int include_obs_index = train_obs[h_index].size()-1;

	// 	vector<double> state(potential_world_model->num_states, 0.0);

	// 	for (int step_index = 0; step_index < (int)train_obs[h_index].size(); step_index++) {
	// 		if (step_index <= include_obs_index) {
	// 			vector<double> starting_state = state;

	// 			for (int n_index = 0; n_index < (int)potential_world_model->obs_networks.size(); n_index++) {
	// 				vector<double> inputs;
	// 				for (int i_index = 0; i_index < (int)potential_world_model->obs_network_inputs[n_index].size(); i_index++) {
	// 					inputs.push_back(starting_state[potential_world_model->obs_network_inputs[n_index][i_index]]);
	// 				}
	// 				inputs.insert(inputs.end(), train_obs[h_index][step_index].begin(),
	// 					train_obs[h_index][step_index].end());
	// 				potential_world_model->obs_networks[n_index]->activate(inputs);
	// 				for (int o_index = 0; o_index < (int)potential_world_model->obs_network_outputs[n_index].size(); o_index++) {
	// 					state[potential_world_model->obs_network_outputs[n_index][o_index]]
	// 						+= potential_world_model->obs_networks[n_index]->output->acti_vals[o_index];
	// 				}
	// 			}
	// 		}

	// 		if (step_index < (int)train_actions[h_index].size()) {
	// 			int action = train_actions[h_index][step_index];

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
	// 		}
	// 	}

	// 	double sum_score = 0.0;
	// 	for (int n_index = 0; n_index < (int)potential_world_model->score_networks.size(); n_index++) {
	// 		vector<double> inputs;
	// 		for (int i_index = 0; i_index < (int)potential_world_model->score_network_inputs[n_index].size(); i_index++) {
	// 			inputs.push_back(state[potential_world_model->score_network_inputs[n_index][i_index]]);
	// 		}
	// 		// temp
	// 		cout << "inputs:";
	// 		for (int i_index = 0; i_index < (int)inputs.size(); i_index++) {
	// 			cout << " " << inputs[i_index];
	// 		}
	// 		cout << endl;
	// 		potential_world_model->score_networks[n_index]->activate(inputs);
	// 		sum_score += potential_world_model->score_networks[n_index]->output->acti_vals[0];
	// 		cout << "potential_world_model->score_networks[n_index]->output->acti_vals[0]: " << potential_world_model->score_networks[n_index]->output->acti_vals[0] << endl;
	// 	}

	// 	cout << "state:";
	// 	for (int s_index = 0; s_index < (int)state.size(); s_index++) {
	// 		cout << " " << state[s_index];
	// 	}
	// 	cout << endl;
	// 	cout << "sum_score: " << sum_score << endl;
	// }
}

double eval_helper(vector<vector<double>>& sample_obs,
				   vector<int>& sample_actions,
				   double sample_target_val,
				   int include_obs_index,
				   WorldModel* world_model,
				   Wrapper* wrapper) {
	vector<double> state(world_model->num_states, 0.0);

	for (int step_index = 0; step_index < (int)sample_obs.size(); step_index++) {
		if (step_index <= include_obs_index) {
			vector<double> starting_state = state;

			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
					inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), sample_obs[step_index].begin(),
					sample_obs[step_index].end());
				world_model->obs_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
					state[world_model->obs_network_outputs[n_index][o_index]]
						+= world_model->obs_networks[n_index]->output->acti_vals[o_index];
				}
			}
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
	for (int n_index = 0; n_index < (int)world_model->score_networks.size(); n_index++) {
		vector<double> inputs;
		for (int i_index = 0; i_index < (int)world_model->score_network_inputs[n_index].size(); i_index++) {
			inputs.push_back(state[world_model->score_network_inputs[n_index][i_index]]);
		}
		world_model->score_networks[n_index]->activate(inputs);
		sum_score += world_model->score_networks[n_index]->output->acti_vals[0];
	}

	return sum_score;
}

void train_helper(Wrapper* wrapper) {
	// temp
	cout << "train_helper" << endl;

	// vector<vector<vector<double>>> selected_obs;
	// vector<vector<int>> selected_actions;
	// vector<double> selected_target_vals;

	// vector<pair<double,int>> misguess_factors(wrapper->sample_obs.size());
	// for (int h_index = 0; h_index < (int)wrapper->sample_obs.size(); h_index++) {
	// 	double misguess = abs(wrapper->sample_target_vals[h_index] - wrapper->sample_predicted_scores[h_index]);
	// 	// misguess_factors[h_index] = {misguess / wrapper->sample_predicted_misguesses[h_index], h_index};
	// 	misguess_factors[h_index] = {misguess, h_index};
	// }
	// sort(misguess_factors.begin(), misguess_factors.end());

	// int num_select = 0.1 * (double)wrapper->sample_obs.size();
	// for (int i_index = 0; i_index < num_select; i_index++) {
	// 	int index = misguess_factors[misguess_factors.size()-1 - i_index].second;
	// 	selected_obs.push_back(wrapper->sample_obs[index]);
	// 	selected_actions.push_back(wrapper->sample_actions[index]);
	// 	selected_target_vals.push_back(wrapper->sample_target_vals[index]);
	// }
	// uniform_int_distribution<int> select_distribution(0, wrapper->sample_obs.size()-1);
	// for (int i_index = 0; i_index < num_select; i_index++) {
	// 	int index = select_distribution(generator);
	// 	selected_obs.push_back(wrapper->sample_obs[index]);
	// 	selected_actions.push_back(wrapper->sample_actions[index]);
	// 	selected_target_vals.push_back(wrapper->sample_target_vals[index]);
	// }

	vector<vector<vector<double>>> selected_obs = wrapper->sample_obs;
	vector<vector<int>> selected_actions = wrapper->sample_actions;
	vector<double> selected_target_vals = wrapper->sample_target_vals;

	vector<vector<vector<double>>> train_obs;
	vector<vector<int>> train_actions;
	vector<double> train_target_vals;
	vector<vector<vector<double>>> verification_obs;
	vector<vector<int>> verification_actions;
	vector<double> verification_target_vals;

	int num_verification = VERIFICATION_RATIO * (double)selected_obs.size();
	int num_train = (int)selected_obs.size() - num_verification;
	{
		vector<int> remaining_indexes(selected_obs.size());
		for (int i_index = 0; i_index < (int)selected_obs.size(); i_index++) {
			remaining_indexes[i_index] = i_index;
		}

		for (int i_index = 0; i_index < num_train; i_index++) {
			uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
			int index = distribution(generator);
			train_obs.push_back(selected_obs[remaining_indexes[index]]);
			train_actions.push_back(selected_actions[remaining_indexes[index]]);
			train_target_vals.push_back(selected_target_vals[remaining_indexes[index]]);

			remaining_indexes.erase(remaining_indexes.begin() + index);
		}
		for (int i_index = 0; i_index < (int)remaining_indexes.size(); i_index++) {
			verification_obs.push_back(selected_obs[remaining_indexes[i_index]]);
			verification_actions.push_back(selected_actions[remaining_indexes[i_index]]);
			verification_target_vals.push_back(selected_target_vals[remaining_indexes[i_index]]);
		}
	}

	uniform_int_distribution<int> train_sample_distribution(0, train_obs.size()-1);
	for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
		int train_sample_index = train_sample_distribution(generator);

		update_world_model_helper(train_obs[train_sample_index],
								  train_actions[train_sample_index],
								  train_target_vals[train_sample_index],
								  wrapper);
	}

	measure_test(wrapper);

	WorldModel* potential_world_model = new WorldModel(wrapper->world_model);

	train_helper(train_obs,
				 train_actions,
				 train_target_vals,
				 potential_world_model,
				 wrapper);

	// temp
	{
		vector<double> existing_misguess(EVAL_ITERS);
		vector<double> new_misguess(EVAL_ITERS);
		uniform_int_distribution<int> train_sample_distribution(0, train_obs.size()-1);
		for (int iter_index = 0; iter_index < EVAL_ITERS; iter_index++) {
			int train_sample_index = train_sample_distribution(generator);

			vector<vector<double>> sample_obs = train_obs[train_sample_index];
			vector<int> sample_actions = train_actions[train_sample_index];
			double sample_target_val = train_target_vals[train_sample_index];

			uniform_int_distribution<int> include_obs_distribution(
				0, sample_obs.size()-1);
			// int include_obs_index = include_obs_distribution(generator);
			int include_obs_index = sample_obs.size()-1;

			double existing_predicted = eval_helper(sample_obs,
													sample_actions,
													sample_target_val,
													include_obs_index,
													wrapper->world_model,
													wrapper);
			existing_misguess[iter_index] = (sample_target_val - existing_predicted)
				* (sample_target_val - existing_predicted);

			double new_predicted = eval_helper(sample_obs,
											   sample_actions,
											   sample_target_val,
											   include_obs_index,
											   potential_world_model,
											   wrapper);
			new_misguess[iter_index] = (sample_target_val - new_predicted)
				* (sample_target_val - new_predicted);
		}

		double existing_sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < EVAL_ITERS; iter_index++) {
			existing_sum_misguess += existing_misguess[iter_index];
		}
		double existing_misguess_average = existing_sum_misguess / EVAL_ITERS;
		cout << "train existing_misguess_average: " << existing_misguess_average << endl;

		double new_sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < EVAL_ITERS; iter_index++) {
			new_sum_misguess += new_misguess[iter_index];
		}
		double new_misguess_average = new_sum_misguess / EVAL_ITERS;
		cout << "new new_misguess_average: " << new_misguess_average << endl;
	}

	vector<double> existing_misguess(EVAL_ITERS);
	vector<double> new_misguess(EVAL_ITERS);
	uniform_int_distribution<int> verification_sample_distribution(0, verification_obs.size()-1);
	for (int iter_index = 0; iter_index < EVAL_ITERS; iter_index++) {
		int verification_sample_index = verification_sample_distribution(generator);

		vector<vector<double>> sample_obs = verification_obs[verification_sample_index];
		vector<int> sample_actions = verification_actions[verification_sample_index];
		double sample_target_val = verification_target_vals[verification_sample_index];

		uniform_int_distribution<int> include_obs_distribution(
			0, sample_obs.size()-1);
		// int include_obs_index = include_obs_distribution(generator);
		int include_obs_index = sample_obs.size()-1;

		double existing_predicted = eval_helper(sample_obs,
												sample_actions,
												sample_target_val,
												include_obs_index,
												wrapper->world_model,
												wrapper);
		existing_misguess[iter_index] = (sample_target_val - existing_predicted)
			* (sample_target_val - existing_predicted);

		double new_predicted = eval_helper(sample_obs,
										   sample_actions,
										   sample_target_val,
										   include_obs_index,
										   potential_world_model,
										   wrapper);
		new_misguess[iter_index] = (sample_target_val - new_predicted)
			* (sample_target_val - new_predicted);
	}

	double existing_sum_misguess = 0.0;
	for (int iter_index = 0; iter_index < EVAL_ITERS; iter_index++) {
		existing_sum_misguess += existing_misguess[iter_index];
	}
	double existing_misguess_average = existing_sum_misguess / EVAL_ITERS;

	double existing_misguess_sum_variance = 0.0;
	for (int iter_index = 0; iter_index < EVAL_ITERS; iter_index++) {
		existing_misguess_sum_variance += (existing_misguess[iter_index] - existing_misguess_average)
			* (existing_misguess[iter_index] - existing_misguess_average);
	}
	double existing_misguess_variance = existing_misguess_sum_variance / EVAL_ITERS;

	double new_sum_misguess = 0.0;
	for (int iter_index = 0; iter_index < EVAL_ITERS; iter_index++) {
		new_sum_misguess += new_misguess[iter_index];
	}
	double new_misguess_average = new_sum_misguess / EVAL_ITERS;

	double new_misguess_sum_variance = 0.0;
	for (int iter_index = 0; iter_index < EVAL_ITERS; iter_index++) {
		new_misguess_sum_variance += (new_misguess[iter_index] - new_misguess_average)
			* (new_misguess[iter_index] - new_misguess_average);
	}
	double new_misguess_variance = new_misguess_sum_variance / EVAL_ITERS;

	double denom = sqrt((existing_misguess_variance + new_misguess_variance) / EVAL_ITERS);
	double t_score = (existing_misguess_average - new_misguess_average) / denom;

	// temp
	cout << "existing_misguess_average: " << existing_misguess_average << endl;
	cout << "existing_misguess_variance: " << existing_misguess_variance << endl;
	cout << "new_misguess_average: " << new_misguess_average << endl;
	cout << "new_misguess_variance: " << new_misguess_variance << endl;
	cout << "t_score: " << t_score << endl;

	#if defined(MDEBUG) && MDEBUG
	if (t_score >= 1.645 || rand()%2 == 0) {
	#else
	if (t_score >= 1.645) {
	#endif /* MDEBUG */
		cout << "add state" << endl;

		delete wrapper->world_model;
		wrapper->world_model = potential_world_model;
	} else {
		delete potential_world_model;
	}

	wrapper->sample_obs.clear();
	wrapper->sample_actions.clear();
	wrapper->sample_target_vals.clear();
	wrapper->sample_predicted_scores.clear();
	wrapper->sample_predicted_misguesses.clear();

	measure_test(wrapper);

	// temp
	cout << "train_helper done" << endl;
}
