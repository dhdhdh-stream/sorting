#include "world_model_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "solution.h"
#include "state_network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int UPDATE_MIN_SAMPLE_SIZE = 10;
const int ITERS_PER_UPDATE = 2;
#else
const int UPDATE_MIN_SAMPLE_SIZE = 1000;
const int ITERS_PER_UPDATE = 100;
#endif /* MDEBUG */

const int STATE_SIZE_HISTORY_NUM_SAVE = 3;

void predict_update_helper(vector<double>& start_state,
						   vector<int>& actions,
						   vector<double>& end_state,
						   WorldModel* world_model,
						   Wrapper* wrapper) {
	vector<double> state = start_state;

	vector<StateNetworkHistory*> network_histories;

	for (int step_index = 0; step_index < (int)actions.size(); step_index++) {
		int action = actions[step_index];

		vector<double> partial_inputs;
		for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
			if (action == a_index) {
				partial_inputs.push_back(1.0);
			} else {
				partial_inputs.push_back(0.0);
			}
		}

		vector<double> inputs;
		inputs.insert(inputs.end(), state.begin(), state.end());
		inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
		StateNetworkHistory* network_history = new StateNetworkHistory();
		world_model->predict_network->activate(inputs,
											   network_history);
		network_histories.push_back(network_history);
		for (int o_index = 0; o_index < world_model->num_states; o_index++) {
			state[o_index] += world_model->predict_network->output->acti_vals[o_index];
		}
	}

	vector<double> state_errors(world_model->num_states);
	for (int i_index = 0; i_index < world_model->num_states; i_index++) {
		state_errors[i_index] = end_state[i_index] - state[i_index];
	}

	for (int step_index = (int)actions.size()-1; step_index >= 0; step_index--) {
		world_model->predict_network->backprop(state_errors,
											   network_histories[step_index]);
		delete network_histories[step_index];
		for (int i_index = 0; i_index < world_model->num_states; i_index++) {
			state_errors[i_index] += world_model->predict_network->input->errors[i_index];
			world_model->predict_network->input->errors[i_index] = 0.0;
		}
	}

	world_model->predict_epoch_iter++;
	if (world_model->predict_epoch_iter >= NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		world_model->predict_network->get_max_update(max_update);

		world_model->predict_average_max_update = 0.999*world_model->predict_average_max_update + 0.001*max_update;

		if (max_update > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / world_model->predict_average_max_update;
			if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
			}

			world_model->predict_network->update_weights(learning_rate);
		}

		world_model->predict_epoch_iter = 0;
	}
}

void true_update_helper(vector<vector<double>>& obs,
						vector<int>& actions,
						double target_val,
						WorldModel* world_model,
						Wrapper* wrapper) {
	int predict_start_index = -1;
	int predict_end_index = -1;
	if (actions.size() > 0) {
		uniform_int_distribution<int> from_start_distribution(0, 1);
		if (from_start_distribution(generator) == 0) {
			uniform_int_distribution<int> start_index_distribution(0, actions.size()-1);
			predict_start_index = start_index_distribution(generator);

			uniform_int_distribution<int> end_index_distribution(predict_start_index+1, obs.size()-1);
			predict_end_index = end_index_distribution(generator);
		} else {
			uniform_int_distribution<int> end_index_distribution(1, obs.size()-1);
			predict_end_index = end_index_distribution(generator);

			uniform_int_distribution<int> start_index_distribution(0, predict_end_index-1);
			predict_start_index = start_index_distribution(generator);
		}
	}

	vector<double> predict_start_state;
	vector<double> predict_end_state;

	vector<double> state(world_model->num_states, 0.0);

	vector<StateNetworkHistory*> obs_network_histories;
	vector<StateNetworkHistory*> action_network_histories;

	for (int step_index = 0; step_index < (int)actions.size(); step_index++) {
		vector<double> obs_inputs;
		obs_inputs.insert(obs_inputs.end(), state.begin(), state.end());
		obs_inputs.insert(obs_inputs.end(), obs[step_index].begin(), obs[step_index].end());
		StateNetworkHistory* obs_network_history = new StateNetworkHistory();
		world_model->obs_network->activate(obs_inputs,
										   obs_network_history);
		obs_network_histories.push_back(obs_network_history);
		for (int o_index = 0; o_index < world_model->num_states; o_index++) {
			state[o_index] += world_model->obs_network->output->acti_vals[o_index];
		}

		if (predict_start_index == step_index) {
			predict_start_state = state;
		}
		if (predict_end_index == step_index) {
			predict_end_state = state;
		}

		int action = actions[step_index];

		vector<double> partial_inputs;
		for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
			if (action == a_index) {
				partial_inputs.push_back(1.0);
			} else {
				partial_inputs.push_back(0.0);
			}
		}

		vector<double> action_inputs;
		action_inputs.insert(action_inputs.end(), state.begin(), state.end());
		action_inputs.insert(action_inputs.end(), partial_inputs.begin(), partial_inputs.end());
		StateNetworkHistory* action_network_history = new StateNetworkHistory();
		world_model->action_network->activate(action_inputs,
											  action_network_history);
		action_network_histories.push_back(action_network_history);
		for (int o_index = 0; o_index < world_model->num_states; o_index++) {
			state[o_index] += world_model->action_network->output->acti_vals[o_index];
		}
	}

	vector<double> obs_inputs;
	obs_inputs.insert(obs_inputs.end(), state.begin(), state.end());
	obs_inputs.insert(obs_inputs.end(), obs.back().begin(), obs.back().end());
	StateNetworkHistory* network_history = new StateNetworkHistory();
	world_model->obs_network->activate(obs_inputs,
									   network_history);
	obs_network_histories.push_back(network_history);
	for (int o_index = 0; o_index < world_model->num_states; o_index++) {
		state[o_index] += world_model->obs_network->output->acti_vals[o_index];
	}

	if (predict_end_index == (int)obs.size()-1) {
		predict_end_state = state;
	}

	world_model->final_network->activate(state);
	double predicted = world_model->final_network->output->acti_vals[0];

	vector<double> final_errors{target_val - predicted};

	double error = abs(target_val - predicted);
	double curr_misguess_variance = (error - world_model->misguess_average) * (error - world_model->misguess_average);
	world_model->misguess_average = 0.9999*world_model->misguess_average + 0.0001*error;
	world_model->misguess_variance_average = 0.9999*world_model->misguess_variance_average + 0.0001*curr_misguess_variance;
	/**
	 * - better than 10000 samples(?)
	 */

	vector<double> state_errors(world_model->num_states, 0.0);

	world_model->final_network->backprop(final_errors);
	for (int i_index = 0; i_index < world_model->num_states; i_index++) {
		state_errors[i_index] += world_model->final_network->input->errors[i_index];
		world_model->final_network->input->errors[i_index] = 0.0;
	}

	world_model->obs_network->backprop(state_errors,
									   obs_network_histories.back());
	delete obs_network_histories.back();
	for (int i_index = 0; i_index < world_model->num_states; i_index++) {
		state_errors[i_index] += world_model->obs_network->input->errors[i_index];
		world_model->obs_network->input->errors[i_index] = 0.0;
	}

	for (int step_index = (int)actions.size()-1; step_index >= 0; step_index--) {
		world_model->action_network->backprop(state_errors,
											  action_network_histories[step_index]);
		delete action_network_histories[step_index];
		for (int i_index = 0; i_index < world_model->num_states; i_index++) {
			state_errors[i_index] += world_model->action_network->input->errors[i_index];
			world_model->action_network->input->errors[i_index] = 0.0;
		}

		world_model->obs_network->backprop(state_errors,
										   obs_network_histories[step_index]);
		delete obs_network_histories[step_index];
		for (int i_index = 0; i_index < world_model->num_states; i_index++) {
			state_errors[i_index] += world_model->obs_network->input->errors[i_index];
			world_model->obs_network->input->errors[i_index] = 0.0;
		}
	}

	world_model->epoch_iter++;
	if (world_model->epoch_iter >= NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		world_model->obs_network->get_max_update(max_update);
		world_model->action_network->get_max_update(max_update);
		world_model->final_network->get_max_update(max_update);

		world_model->average_max_update = 0.999*world_model->average_max_update + 0.001*max_update;

		if (max_update > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / world_model->average_max_update;
			if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
			}

			world_model->obs_network->update_weights(learning_rate);
			world_model->action_network->update_weights(learning_rate);
			world_model->final_network->update_weights(learning_rate);
		}

		world_model->epoch_iter = 0;
	}

	if (actions.size() > 0) {
		vector<int> predict_actions = vector<int>(actions.begin() + predict_start_index,
			actions.begin() + predict_end_index);

		predict_update_helper(predict_start_state,
							  predict_actions,
							  predict_end_state,
							  world_model,
							  wrapper);
	}
}

void update_world_model_helper(Wrapper* wrapper) {
	if (wrapper->sample_obs.size() >= UPDATE_MIN_SAMPLE_SIZE) {
		uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);

		for (int iter_index = 0; iter_index < ITERS_PER_UPDATE; iter_index++) {
			int sample_index = sample_distribution(generator);
			true_update_helper(wrapper->sample_obs[sample_index],
							   wrapper->sample_actions[sample_index],
							   wrapper->sample_target_vals[sample_index],
							   wrapper->curr_model,
							   wrapper);
		}

		for (int iter_index = 0; iter_index < ITERS_PER_UPDATE; iter_index++) {
			int sample_index = sample_distribution(generator);
			true_update_helper(wrapper->sample_obs[sample_index],
							   wrapper->sample_actions[sample_index],
							   wrapper->sample_target_vals[sample_index],
							   wrapper->large_model,
							   wrapper);
		}
	}
}

void check_state_size_helper(Wrapper* wrapper) {
	wrapper->curr_misguess_average_history.push_back(wrapper->curr_model->misguess_average);
	wrapper->curr_misguess_variance_average_history.push_back(wrapper->curr_model->misguess_variance_average);
	wrapper->large_misguess_average_history.push_back(wrapper->large_model->misguess_average);
	wrapper->large_misguess_variance_average_history.push_back(wrapper->large_model->misguess_variance_average);
	if (wrapper->curr_misguess_average_history.size() > STATE_SIZE_HISTORY_NUM_SAVE) {
		wrapper->curr_misguess_average_history.erase(wrapper->curr_misguess_average_history.begin());
		wrapper->curr_misguess_variance_average_history.erase(wrapper->curr_misguess_variance_average_history.begin());
		wrapper->large_misguess_average_history.erase(wrapper->large_misguess_average_history.begin());
		wrapper->large_misguess_variance_average_history.erase(wrapper->large_misguess_variance_average_history.begin());
	}

	if (wrapper->curr_misguess_average_history.size() >= STATE_SIZE_HISTORY_NUM_SAVE) {
		bool should_increase = true;
		for (int i_index = 0; i_index < STATE_SIZE_HISTORY_NUM_SAVE; i_index++) {
			for (int j_index = 0; j_index < STATE_SIZE_HISTORY_NUM_SAVE; j_index++) {
				double denom = sqrt((wrapper->large_misguess_variance_average_history[j_index]
					+ wrapper->curr_misguess_variance_average_history[i_index]) / 10000.0);
				double t_score = (wrapper->curr_misguess_average_history[i_index]
					- wrapper->large_misguess_average_history[j_index]) / denom;

				// temp
				cout << i_index<< " " << j_index << ": " << t_score << endl;

				if (t_score < 2.326) {
					should_increase = false;
				}
			}
		}

		if (should_increase) {
			delete wrapper->curr_model;
			wrapper->curr_model = wrapper->large_model;

			wrapper->large_model = new WorldModel(wrapper->curr_model);
			wrapper->large_model->add_states();

			wrapper->solution->pad_new_state(NUM_STATE_CHANGE);

			wrapper->curr_misguess_average_history.clear();
			wrapper->curr_misguess_variance_average_history.clear();
			wrapper->large_misguess_average_history.clear();
			wrapper->large_misguess_variance_average_history.clear();
		}
	}
}
