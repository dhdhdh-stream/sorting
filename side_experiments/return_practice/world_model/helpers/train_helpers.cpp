#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "solution.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

const int NUM_EXISTING_STATES = 8;
const int NUM_NEW_STATES = 2;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
const int EVAL_ITERS = 10;
#else
const int TRAIN_ITERS = 300000;
const int EVAL_ITERS = 4000;
#endif /* MDEBUG */

const double VERIFICATION_RATIO = 0.2;

const int TRAIN_TRY_ITERS = 2;

const double SAMPLE_SAVE_RATIO = 0.1;

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
	Network* new_final_network = new Network(NUM_NEW_STATES + new_final_existing_inputs.size(), 1);

	uniform_int_distribution<int> sample_distribution(0, train_obs.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
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
	for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
		vector<double> inputs;
		for (int i_index = 0; i_index < (int)world_model->final_network_inputs[n_index].size(); i_index++) {
			inputs.push_back(state[world_model->final_network_inputs[n_index][i_index]]);
		}
		world_model->final_networks[n_index]->activate(inputs);
		sum_score += world_model->final_networks[n_index]->output->acti_vals[0];
	}

	return sum_score;
}

void train_helper(Wrapper* wrapper) {
	// temp
	cout << "train_helper" << endl;

	for (int try_index = 0; try_index < TRAIN_TRY_ITERS; try_index++) {
		cout << "try_index: " << try_index << endl;

		vector<vector<vector<double>>> train_obs;
		vector<vector<int>> train_actions;
		vector<double> train_target_vals;
		vector<vector<vector<double>>> verification_obs;
		vector<vector<int>> verification_actions;
		vector<double> verification_target_vals;

		int num_new_verification = VERIFICATION_RATIO * (double)wrapper->new_sample_obs.size();
		int num_new_train = (int)wrapper->new_sample_obs.size() - num_new_verification;
		{
			vector<int> remaining_indexes(wrapper->new_sample_obs.size());
			for (int i_index = 0; i_index < (int)wrapper->new_sample_obs.size(); i_index++) {
				remaining_indexes[i_index] = i_index;
			}

			for (int i_index = 0; i_index < num_new_train; i_index++) {
				uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
				int index = distribution(generator);
				train_obs.push_back(wrapper->new_sample_obs[remaining_indexes[index]]);
				train_actions.push_back(wrapper->new_sample_actions[remaining_indexes[index]]);
				train_target_vals.push_back(wrapper->new_sample_target_vals[remaining_indexes[index]]);

				remaining_indexes.erase(remaining_indexes.begin() + index);
			}
			for (int i_index = 0; i_index < (int)remaining_indexes.size(); i_index++) {
				verification_obs.push_back(wrapper->new_sample_obs[remaining_indexes[i_index]]);
				verification_actions.push_back(wrapper->new_sample_actions[remaining_indexes[i_index]]);
				verification_target_vals.push_back(wrapper->new_sample_target_vals[remaining_indexes[i_index]]);
			}
		}

		int num_old_verification;
		int num_old_train;
		if (wrapper->new_sample_obs.size() >= wrapper->old_sample_obs.size()) {
			num_old_verification = VERIFICATION_RATIO * (double)wrapper->old_sample_obs.size();
			num_old_train = (int)wrapper->old_sample_obs.size() - num_old_verification;
		} else {
			num_old_verification = num_new_verification;
			num_old_train = num_new_train;
		}
		{
			vector<int> remaining_indexes(wrapper->old_sample_obs.size());
			for (int i_index = 0; i_index < (int)wrapper->old_sample_obs.size(); i_index++) {
				remaining_indexes[i_index] = i_index;
			}

			for (int i_index = 0; i_index < num_old_train; i_index++) {
				uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
				int index = distribution(generator);
				train_obs.push_back(wrapper->old_sample_obs[remaining_indexes[index]]);
				train_actions.push_back(wrapper->old_sample_actions[remaining_indexes[index]]);
				train_target_vals.push_back(wrapper->old_sample_target_vals[remaining_indexes[index]]);

				remaining_indexes.erase(remaining_indexes.begin() + index);
			}
			for (int i_index = 0; i_index < num_old_verification; i_index++) {
				uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
				int index = distribution(generator);
				verification_obs.push_back(wrapper->old_sample_obs[remaining_indexes[index]]);
				verification_actions.push_back(wrapper->old_sample_actions[remaining_indexes[index]]);
				verification_target_vals.push_back(wrapper->old_sample_target_vals[remaining_indexes[index]]);

				remaining_indexes.erase(remaining_indexes.begin() + index);
			}
		}

		WorldModel* potential_world_model = new WorldModel(wrapper->world_model);

		train_helper(train_obs,
					 train_actions,
					 train_target_vals,
					 potential_world_model,
					 wrapper);

		double existing_sum_misguess = 0.0;
		double new_sum_misguess = 0.0;
		uniform_int_distribution<int> verification_sample_distribution(0, verification_obs.size()-1);
		for (int iter_index = 0; iter_index < EVAL_ITERS; iter_index++) {
			int verification_sample_index = verification_sample_distribution(generator);

			vector<vector<double>> sample_obs = verification_obs[verification_sample_index];
			vector<int> sample_actions = verification_actions[verification_sample_index];
			double sample_target_val = verification_target_vals[verification_sample_index];

			uniform_int_distribution<int> include_obs_distribution(
				0, sample_obs.size()-1);
			int include_obs_index = include_obs_distribution(generator);

			double existing_predicted = eval_helper(sample_obs,
													sample_actions,
													sample_target_val,
													include_obs_index,
													wrapper->world_model,
													wrapper);
			existing_sum_misguess += (sample_target_val - existing_predicted)
				* (sample_target_val - existing_predicted);

			double new_predicted = eval_helper(sample_obs,
											   sample_actions,
											   sample_target_val,
											   include_obs_index,
											   potential_world_model,
											   wrapper);
			new_sum_misguess += (sample_target_val - new_predicted)
				* (sample_target_val - new_predicted);
		}

		// temp
		cout << "existing_sum_misguess: " << existing_sum_misguess << endl;
		cout << "new_sum_misguess: " << new_sum_misguess << endl;

		#if defined(MDEBUG) && MDEBUG
		if (new_sum_misguess < existing_sum_misguess || rand()%2 == 0) {
		#else
		if (new_sum_misguess < existing_sum_misguess) {
		#endif /* MDEBUG */
			cout << "add state" << endl;

			delete wrapper->world_model;
			wrapper->world_model = potential_world_model;

			wrapper->solution->pad_new_state(NUM_NEW_STATES);
		} else {
			delete potential_world_model;
		}
	}

	int num_save = SAMPLE_SAVE_RATIO * (double)wrapper->new_sample_obs.size();
	for (int i_index = 0; i_index < num_save; i_index++) {
		uniform_int_distribution<int> distribution(0, wrapper->new_sample_obs.size()-1);
		int index = distribution(generator);

		wrapper->old_sample_obs.push_back(wrapper->new_sample_obs[index]);
		wrapper->new_sample_obs.erase(wrapper->new_sample_obs.begin() + index);
		wrapper->old_sample_actions.push_back(wrapper->new_sample_actions[index]);
		wrapper->new_sample_actions.erase(wrapper->new_sample_actions.begin() + index);
		wrapper->old_sample_target_vals.push_back(wrapper->new_sample_target_vals[index]);
		wrapper->new_sample_target_vals.erase(wrapper->new_sample_target_vals.begin() + index);
	}

	wrapper->new_sample_obs.clear();
	wrapper->new_sample_actions.clear();
	wrapper->new_sample_target_vals.clear();

	// temp
	cout << "train_helper done" << endl;
}
