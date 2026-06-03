#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"

using namespace std;

void update_world_model_helper(vector<vector<double>>& obs,
							   vector<int>& actions,
							   double target_val,
							   double& error,
							   WorldModel* world_model) {
	uniform_int_distribution<int> include_obs_distribution(0, obs.size()-1);
	// int include_obs_index = include_obs_distribution(generator);
	int include_obs_index = obs.size()-1;

	vector<double> state(world_model->num_states, 0.0);

	vector<NetworkHistory*> obs_network_histories;
	vector<NetworkHistory*> action_network_histories;

	for (int step_index = 0; step_index < (int)obs.size(); step_index++) {
		if (step_index <= include_obs_index) {
			vector<double> inputs;
			inputs.insert(inputs.end(), state.begin(), state.end());
			inputs.insert(inputs.end(), obs[step_index].begin(), obs[step_index].end());
			NetworkHistory* network_history = new NetworkHistory();
			world_model->obs_network->activate(inputs,
											   network_history);
			obs_network_histories.push_back(network_history);
			for (int o_index = 0; o_index < world_model->num_states; o_index++) {
				state[o_index] += world_model->obs_network->output->acti_vals[o_index];
			}
		}

		if (step_index < (int)actions.size()) {
			int action = actions[step_index];

			vector<double> partial_inputs;
			for (int a_index = 0; a_index < world_model->num_actions; a_index++) {
				if (action == a_index) {
					partial_inputs.push_back(1.0);
				} else {
					partial_inputs.push_back(0.0);
				}
			}

			vector<double> inputs;
			inputs.insert(inputs.end(), state.begin(), state.end());
			inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
			NetworkHistory* network_history = new NetworkHistory();
			world_model->action_network->activate(inputs,
												  network_history);
			action_network_histories.push_back(network_history);
			for (int o_index = 0; o_index < world_model->num_states; o_index++) {
				state[o_index] += world_model->action_network->output->acti_vals[o_index];
			}
		}
	}

	world_model->score_network->activate(state);

	vector<double> score_errors{target_val - world_model->score_network->output->acti_vals[0]};

	error = abs(target_val - world_model->score_network->output->acti_vals[0]);

	double curr_misguess_variance = (error - world_model->misguess_average) * (error - world_model->misguess_average);
	world_model->misguess_average = 0.9999*world_model->misguess_average + 0.0001*error;
	world_model->misguess_variance_average = 0.9999*world_model->misguess_variance_average + 0.0001*curr_misguess_variance;
	/**
	 * - better than 10000 samples(?)
	 */

	vector<double> state_errors(world_model->num_states);

	world_model->score_network->backprop(score_errors);
	for (int i_index = 0; i_index < world_model->num_states; i_index++) {
		state_errors[i_index] = world_model->score_network->input->errors[i_index];
		world_model->score_network->input->errors[i_index] = 0.0;
	}

	for (int step_index = (int)obs.size()-1; step_index >= 0; step_index--) {
		if (step_index < (int)actions.size()) {
			world_model->action_network->backprop(state_errors,
												  action_network_histories[step_index]);
			delete action_network_histories[step_index];
			for (int i_index = 0; i_index < world_model->num_states; i_index++) {
				state_errors[i_index] += world_model->action_network->input->errors[i_index];
				world_model->action_network->input->errors[i_index] = 0.0;
			}
		}

		if (step_index <= include_obs_index) {
			world_model->obs_network->backprop(state_errors,
											   obs_network_histories[step_index]);
			delete obs_network_histories[step_index];
			for (int i_index = 0; i_index < world_model->num_states; i_index++) {
				state_errors[i_index] += world_model->obs_network->input->errors[i_index];
				world_model->obs_network->input->errors[i_index] = 0.0;
			}
		}
	}

	world_model->epoch_iter++;
	if (world_model->epoch_iter >= NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		world_model->obs_network->get_max_update(max_update);
		world_model->action_network->get_max_update(max_update);
		world_model->score_network->get_max_update(max_update);

		world_model->average_max_update = 0.999*world_model->average_max_update + 0.001*max_update;

		if (max_update > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / world_model->average_max_update;
			if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
			}

			world_model->obs_network->update_weights(learning_rate);
			world_model->action_network->update_weights(learning_rate);
			world_model->score_network->update_weights(learning_rate);
		}

		world_model->epoch_iter = 0;
	}
}
