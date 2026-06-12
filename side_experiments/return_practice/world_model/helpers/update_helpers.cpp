#include "world_model_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "predict_wrapper.h"
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
const int ITERS_PER_UPDATE = 1000;
#endif /* MDEBUG */

const int ITERS_PER_PREDICT = 40;

#if defined(MDEBUG) && MDEBUG
const int PREDICT_CANDIDATE_CHECK_NUM_ITERS = 100;
#else
const int PREDICT_CANDIDATE_CHECK_NUM_ITERS = 2000000;
/**
 * - actual num iters divided by ITERS_PER_PREDICT
 */
#endif /* MDEBUG */

const int STATE_SIZE_HISTORY_NUM_SAVE = 3;

void predict_update_helper(vector<double>& start_state,
						   vector<double>& end_state,
						   PredictWrapper* predict_wrapper,
						   Wrapper* wrapper) {
	vector<double> diff(start_state.size());
	for (int s_index = 0; s_index < (int)start_state.size(); s_index++) {
		diff[s_index] = end_state[s_index] - start_state[s_index];
	}

	// temp
	if ((wrapper->world_model->curr_candidate_iter+1) % 500000 == 0) {
		cout << "diff:";
		for (int d_index = 0; d_index < (int)diff.size(); d_index++) {
			cout << " " << diff[d_index];
		}
		cout << endl;
	}

	{
		predict_wrapper->average_network->activate(start_state);
		vector<double> errors((int)start_state.size());
		for (int s_index = 0; s_index < (int)start_state.size(); s_index++) {
			errors[s_index] = diff[s_index] - predict_wrapper->average_network->output->acti_vals[s_index];
		}
		predict_wrapper->average_network->backprop(errors);

		predict_wrapper->average_epoch_iter++;
		if (predict_wrapper->average_epoch_iter >= NETWORK_EPOCH_SIZE) {
			double max_update = 0.0;
			predict_wrapper->average_network->get_max_update(max_update);

			predict_wrapper->average_average_max_update = 0.999*predict_wrapper->average_average_max_update + 0.001*max_update;

			if (max_update > 0.0) {
				double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / predict_wrapper->average_average_max_update;
				if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
					learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
				}

				predict_wrapper->average_network->update_weights(learning_rate);
			}

			predict_wrapper->average_epoch_iter = 0;
		}
	}

	{
		int min_index;
		double min_error = numeric_limits<double>::max();
		for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
			predict_wrapper->val_networks[n_index]->activate(start_state);

			double error = 0.0;
			for (int s_index = 0; s_index < (int)start_state.size(); s_index++) {
				error += (diff[s_index] - predict_wrapper->val_networks[n_index]->output->acti_vals[s_index])
					* (diff[s_index] - predict_wrapper->val_networks[n_index]->output->acti_vals[s_index]);
			}
			if (error < min_error) {
				min_index = n_index;
				min_error = error;
			}
		}

		predict_wrapper->misguess_average = 0.9999*predict_wrapper->misguess_average + 0.0001*min_error;

		{
			// temp
			if ((wrapper->world_model->curr_candidate_iter+1) % 500000 == 0) {
				cout << "predict_wrapper->val_networks[min_index]->output->acti_vals:";
				for (int d_index = 0; d_index < (int)predict_wrapper->val_networks[min_index]->output->acti_vals.size(); d_index++) {
					cout << " " << predict_wrapper->val_networks[min_index]->output->acti_vals[d_index];
				}
				cout << endl;
				cout << "min_index: " << min_index << endl;
			}

			vector<double> errors((int)start_state.size());
			for (int s_index = 0; s_index < (int)start_state.size(); s_index++) {
				errors[s_index] = diff[s_index] - predict_wrapper->val_networks[min_index]->output->acti_vals[s_index];
			}
			predict_wrapper->val_networks[min_index]->backprop(errors);

			predict_wrapper->val_epoch_iters[min_index]++;
			if (predict_wrapper->val_epoch_iters[min_index] >= NETWORK_EPOCH_SIZE) {
				double max_update = 0.0;
				predict_wrapper->val_networks[min_index]->get_max_update(max_update);

				predict_wrapper->val_average_max_updates[min_index] = 0.999*predict_wrapper->val_average_max_updates[min_index] + 0.001*max_update;

				if (max_update > 0.0) {
					double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / predict_wrapper->val_average_max_updates[min_index];
					if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
						learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
					}

					predict_wrapper->val_networks[min_index]->update_weights(learning_rate);
				}

				predict_wrapper->val_epoch_iters[min_index] = 0;
			}
		}

		vector<double> select_vals(NUM_PREDICT);
		for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
			predict_wrapper->select_networks[n_index]->activate(start_state);
			select_vals[n_index] = predict_wrapper->select_networks[n_index]->output->acti_vals[0];
		}

		double max_select_val = select_vals[0];
		for (int n_index = 1; n_index < NUM_PREDICT; n_index++) {
			if (select_vals[n_index] > max_select_val) {
				max_select_val = select_vals[n_index];
			}
		}
		for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
			select_vals[n_index] -= max_select_val;
		}

		for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
			select_vals[n_index] = exp(select_vals[n_index]);
		}

		double sum_select = 0.0;
		for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
			sum_select += select_vals[n_index];
		}
		for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
			select_vals[n_index] /= sum_select;
		}

		// temp
		if ((wrapper->world_model->curr_candidate_iter+1) % 500000 == 0) {
			cout << "select_vals:";
			for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
				cout << " " << select_vals[n_index];
			}
			cout << endl;
		}

		for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
			if (n_index == min_index) {
				double error = 1.0 - select_vals[n_index];
				predict_wrapper->select_networks[n_index]->backprop(error);
			} else {
				double error = -select_vals[n_index];
				predict_wrapper->select_networks[n_index]->backprop(error);
			}
		}
	}
}

void update_helper(vector<vector<double>>& obs,
				   vector<int>& actions,
				   double target_val,
				   Wrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

	vector<double> state(world_model->num_states, 0.0);
	vector<double> large_state(NUM_STATE_CHANGE, 0.0);

	vector<vector<StateNetworkHistory*>> obs_network_histories;
	vector<vector<StateNetworkHistory*>> action_network_histories;
	vector<StateNetworkHistory*> large_obs_network_histories;
	vector<StateNetworkHistory*> large_action_network_histories;

	for (int step_index = 0; step_index < (int)obs.size(); step_index++) {
		{
			vector<double> start_state = state;
			vector<double> start_large_state = large_state;

			vector<StateNetworkHistory*> step_obs_network_histories;
			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(start_state[world_model->network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), obs[step_index].begin(), obs[step_index].end());
				StateNetworkHistory* network_history = new StateNetworkHistory();
				world_model->obs_networks[n_index]->activate(inputs,
															 network_history);
				step_obs_network_histories.push_back(network_history);
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					state[world_model->network_outputs[n_index][o_index]] +=
						world_model->obs_networks[n_index]->output->acti_vals[o_index];
				}
			}
			obs_network_histories.push_back(step_obs_network_histories);

			{
				vector<double> inputs;
				inputs.insert(inputs.end(), start_state.begin(), start_state.end());
				inputs.insert(inputs.end(), start_large_state.begin(), start_large_state.end());
				inputs.insert(inputs.end(), obs[step_index].begin(), obs[step_index].end());
				StateNetworkHistory* network_history = new StateNetworkHistory();
				world_model->large_obs_network->activate(inputs,
														 network_history);
				large_obs_network_histories.push_back(network_history);
				for (int o_index = 0; o_index < NUM_STATE_CHANGE; o_index++) {
					large_state[o_index] += world_model->large_obs_network->output->acti_vals[o_index];
				}
			}

			world_model->curr_candidate_iter++;
			if ((world_model->curr_candidate_iter+1) % ITERS_PER_PREDICT == 0) {
				predict_update_helper(start_state,
									  state,
									  world_model->curr_predict,
									  wrapper);
				predict_update_helper(start_state,
									  state,
									  world_model->curr_candidate_predict,
									  wrapper);
			}
			world_model->large_candidate_iter++;
			if ((world_model->large_candidate_iter+1) % ITERS_PER_PREDICT == 0) {
				vector<double> combined_start_state;
				combined_start_state.insert(combined_start_state.end(), start_state.begin(), start_state.end());
				combined_start_state.insert(combined_start_state.end(), start_large_state.begin(), start_large_state.end());
				vector<double> combined_state;
				combined_state.insert(combined_state.end(), state.begin(), state.end());
				combined_state.insert(combined_state.end(), large_state.begin(), large_state.end());

				predict_update_helper(combined_start_state,
									  combined_state,
									  world_model->large_predict,
									  wrapper);
				predict_update_helper(combined_start_state,
									  combined_state,
									  world_model->large_candidate_predict,
									  wrapper);
			}
		}

		if (step_index < (int)actions.size()) {
			int action = actions[step_index];

			vector<double> partial_inputs;
			for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
				if (action == a_index) {
					partial_inputs.push_back(1.0);
				} else {
					partial_inputs.push_back(0.0);
				}
			}

			vector<double> start_state = state;
			vector<double> start_large_state = large_state;

			vector<StateNetworkHistory*> step_action_network_histories;
			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(start_state[world_model->network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
				StateNetworkHistory* network_history = new StateNetworkHistory();
				world_model->action_networks[n_index]->activate(inputs,
																network_history);
				step_action_network_histories.push_back(network_history);
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					state[world_model->network_outputs[n_index][o_index]] +=
						world_model->action_networks[n_index]->output->acti_vals[o_index];
				}
			}
			action_network_histories.push_back(step_action_network_histories);

			{
				vector<double> inputs;
				inputs.insert(inputs.end(), start_state.begin(), start_state.end());
				inputs.insert(inputs.end(), start_large_state.begin(), start_large_state.end());
				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
				StateNetworkHistory* network_history = new StateNetworkHistory();
				world_model->large_action_network->activate(inputs,
															network_history);
				large_action_network_histories.push_back(network_history);
				for (int o_index = 0; o_index < NUM_STATE_CHANGE; o_index++) {
					large_state[o_index] += world_model->large_action_network->output->acti_vals[o_index];
				}
			}
		}
	}

	world_model->curr_final_network->activate(state);
	double curr_predicted = world_model->curr_final_network->output->acti_vals[0];

	vector<double> curr_final_errors{target_val - curr_predicted};

	double curr_error = abs(target_val - curr_predicted);
	double curr_run_misguess_variance = (curr_error - world_model->curr_misguess_average) * (curr_error - world_model->curr_misguess_average);
	world_model->curr_misguess_average = 0.9999*world_model->curr_misguess_average + 0.0001*curr_error;
	world_model->curr_misguess_variance_average = 0.9999*world_model->curr_misguess_variance_average + 0.0001*curr_run_misguess_variance;
	/**
	 * - better than 10000 samples(?)
	 */

	vector<double> state_errors(world_model->num_states, 0.0);

	world_model->curr_final_network->backprop(curr_final_errors);
	for (int i_index = 0; i_index < world_model->num_states; i_index++) {
		state_errors[i_index] += world_model->curr_final_network->input->errors[i_index];
		world_model->curr_final_network->input->errors[i_index] = 0.0;
	}

	vector<double> combined_state;
	combined_state.insert(combined_state.end(), state.begin(), state.end());
	combined_state.insert(combined_state.end(), large_state.begin(), large_state.end());

	world_model->large_final_network->activate(combined_state);
	double large_predicted = world_model->large_final_network->output->acti_vals[0];

	vector<double> large_final_errors{target_val - large_predicted};

	double large_error = abs(target_val - large_predicted);
	double large_run_misguess_variance = (large_error - world_model->large_misguess_average) * (large_error - world_model->large_misguess_average);
	world_model->large_misguess_average = 0.9999*world_model->large_misguess_average + 0.0001*large_error;
	world_model->large_misguess_variance_average = 0.9999*world_model->large_misguess_variance_average + 0.0001*large_run_misguess_variance;

	vector<double> large_state_errors(NUM_STATE_CHANGE, 0.0);

	world_model->large_final_network->backprop(large_final_errors);
	for (int i_index = 0; i_index < NUM_STATE_CHANGE; i_index++) {
		large_state_errors[i_index] += world_model->large_final_network->input->errors[world_model->num_states + i_index];
		world_model->large_final_network->input->errors[world_model->num_states + i_index] = 0.0;
	}

	for (int step_index = (int)obs.size()-1; step_index >= 0; step_index--) {
		if (step_index < (int)actions.size()) {
			vector<double> start_errors = state_errors;

			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
				vector<double> errors;
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					errors.push_back(start_errors[world_model->network_outputs[n_index][o_index]]);
				}
				world_model->action_networks[n_index]->backprop(errors,
																action_network_histories[step_index][n_index]);
				delete action_network_histories[step_index][n_index];
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					state_errors[world_model->network_inputs[n_index][i_index]] +=
						world_model->action_networks[n_index]->input->errors[i_index];
					world_model->action_networks[n_index]->input->errors[i_index] = 0.0;
				}
			}

			{
				world_model->large_action_network->backprop(large_state_errors,
															large_action_network_histories[step_index]);
				delete large_action_network_histories[step_index];
				for (int i_index = 0; i_index < NUM_STATE_CHANGE; i_index++) {
					large_state_errors[i_index] += world_model->large_action_network->input->errors[world_model->num_states + i_index];
					world_model->large_action_network->input->errors[world_model->num_states + i_index] = 0.0;
				}
			}
		}

		{
			vector<double> start_errors = state_errors;

			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> errors;
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					errors.push_back(start_errors[world_model->network_outputs[n_index][o_index]]);
				}
				world_model->obs_networks[n_index]->backprop(errors,
															 obs_network_histories[step_index][n_index]);
				delete obs_network_histories[step_index][n_index];
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					state_errors[world_model->network_inputs[n_index][i_index]] +=
						world_model->obs_networks[n_index]->input->errors[i_index];
					world_model->obs_networks[n_index]->input->errors[i_index] = 0.0;
				}
			}

			{
				world_model->large_obs_network->backprop(large_state_errors,
														 large_obs_network_histories[step_index]);
				delete large_obs_network_histories[step_index];
				for (int i_index = 0; i_index < NUM_STATE_CHANGE; i_index++) {
					large_state_errors[i_index] += world_model->large_obs_network->input->errors[world_model->num_states + i_index];
					world_model->large_obs_network->input->errors[world_model->num_states + i_index] = 0.0;
				}
			}
		}
	}

	world_model->curr_epoch_iter++;
	if (world_model->curr_epoch_iter >= NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
			world_model->obs_networks[n_index]->get_max_update(max_update);
		}
		for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
			world_model->action_networks[n_index]->get_max_update(max_update);
		}
		world_model->curr_final_network->get_max_update(max_update);

		world_model->curr_average_max_update = 0.999*world_model->curr_average_max_update + 0.001*max_update;

		if (max_update > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / world_model->curr_average_max_update;
			if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
			}

			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				world_model->obs_networks[n_index]->update_weights(learning_rate);
			}
			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
				world_model->action_networks[n_index]->update_weights(learning_rate);
			}
			world_model->curr_final_network->update_weights(learning_rate);
		}

		world_model->curr_epoch_iter = 0;
	}

	world_model->large_epoch_iter++;
	if (world_model->large_epoch_iter >= NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		world_model->large_obs_network->get_max_update(max_update);
		world_model->large_action_network->get_max_update(max_update);
		world_model->large_final_network->get_max_update(max_update);

		world_model->large_average_max_update = 0.999*world_model->large_average_max_update + 0.001*max_update;

		if (max_update > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / world_model->large_average_max_update;
			if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
			}

			world_model->large_obs_network->update_weights(learning_rate);
			world_model->large_action_network->update_weights(learning_rate);
			world_model->large_final_network->update_weights(learning_rate);
		}

		world_model->large_epoch_iter = 0;
	}

	if (world_model->curr_candidate_iter >= PREDICT_CANDIDATE_CHECK_NUM_ITERS) {
		if (world_model->curr_candidate_predict->misguess_average < world_model->curr_predict->misguess_average) {
			delete world_model->curr_predict;
			world_model->curr_predict = world_model->curr_candidate_predict;
		} else {
			delete world_model->curr_candidate_predict;
		}

		world_model->curr_candidate_predict = new PredictWrapper(world_model->curr_predict);
		world_model->curr_candidate_predict->twiddle();

		world_model->curr_candidate_iter = 0;
	}
	if (world_model->large_candidate_iter >= PREDICT_CANDIDATE_CHECK_NUM_ITERS) {
		if (world_model->large_candidate_predict->misguess_average < world_model->large_predict->misguess_average) {
			delete world_model->large_predict;
			world_model->large_predict = world_model->large_candidate_predict;
		} else {
			delete world_model->large_candidate_predict;
		}

		world_model->large_candidate_predict = new PredictWrapper(world_model->large_predict);
		world_model->large_candidate_predict->twiddle();

		world_model->large_candidate_iter = 0;
	}
}

void update_world_model_helper(Wrapper* wrapper) {
	if (wrapper->sample_obs.size() >= UPDATE_MIN_SAMPLE_SIZE) {
		uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);

		for (int iter_index = 0; iter_index < ITERS_PER_UPDATE; iter_index++) {
			int sample_index = sample_distribution(generator);
			update_helper(wrapper->sample_obs[sample_index],
						  wrapper->sample_actions[sample_index],
						  wrapper->sample_target_vals[sample_index],
						  wrapper);
		}
	}
}

void check_state_size_helper(Wrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

	world_model->curr_misguess_average_history.push_back(world_model->curr_misguess_average);
	world_model->curr_misguess_variance_average_history.push_back(world_model->curr_misguess_variance_average);
	world_model->large_misguess_average_history.push_back(world_model->large_misguess_average);
	world_model->large_misguess_variance_average_history.push_back(world_model->large_misguess_variance_average);
	if (world_model->curr_misguess_average_history.size() > STATE_SIZE_HISTORY_NUM_SAVE) {
		world_model->curr_misguess_average_history.erase(world_model->curr_misguess_average_history.begin());
		world_model->curr_misguess_variance_average_history.erase(world_model->curr_misguess_variance_average_history.begin());
		world_model->large_misguess_average_history.erase(world_model->large_misguess_average_history.begin());
		world_model->large_misguess_variance_average_history.erase(world_model->large_misguess_variance_average_history.begin());
	}

	if (world_model->curr_misguess_average_history.size() >= STATE_SIZE_HISTORY_NUM_SAVE) {
		bool should_increase = true;
		for (int i_index = 0; i_index < STATE_SIZE_HISTORY_NUM_SAVE; i_index++) {
			for (int j_index = 0; j_index < STATE_SIZE_HISTORY_NUM_SAVE; j_index++) {
				double denom = sqrt((world_model->large_misguess_variance_average_history[j_index]
					+ world_model->curr_misguess_variance_average_history[i_index]) / 10000.0);
				double t_score = (world_model->curr_misguess_average_history[i_index]
					- world_model->large_misguess_average_history[j_index]) / denom;

				// // temp
				// cout << i_index<< " " << j_index << ": " << t_score << endl;

				if (t_score < 2.326) {
					should_increase = false;
				}
			}
		}

		if (should_increase) {
			vector<int> new_network_inputs;
			for (int i_index = 0; i_index < world_model->num_states + NUM_STATE_CHANGE; i_index++) {
				new_network_inputs.push_back(i_index);
			}
			world_model->network_inputs.push_back(new_network_inputs);
			vector<int> new_network_outputs;
			for (int o_index = 0; o_index < NUM_STATE_CHANGE; o_index++) {
				new_network_outputs.push_back(world_model->num_states + o_index);
			}
			world_model->network_outputs.push_back(new_network_outputs);
			for (int i_index = 0; i_index < (int)world_model->large_obs_network->input->errors.size(); i_index++) {
				world_model->large_obs_network->input->errors[i_index] = 0.0;
			}
			world_model->obs_networks.push_back(world_model->large_obs_network);
			for (int i_index = 0; i_index < (int)world_model->large_action_network->input->errors.size(); i_index++) {
				world_model->large_action_network->input->errors[i_index] = 0.0;
			}
			world_model->action_networks.push_back(world_model->large_action_network);

			delete world_model->curr_final_network;
			for (int i_index = 0; i_index < (int)world_model->large_final_network->input->errors.size(); i_index++) {
				world_model->large_final_network->input->errors[i_index] = 0.0;
			}
			world_model->curr_final_network = world_model->large_final_network;

			world_model->curr_epoch_iter = world_model->large_epoch_iter;
			world_model->curr_average_max_update = world_model->large_average_max_update;

			world_model->curr_misguess_average = world_model->large_misguess_average;
			world_model->curr_misguess_variance_average = world_model->large_misguess_variance_average;

			delete world_model->curr_predict;
			world_model->curr_predict = world_model->large_predict;

			delete world_model->curr_candidate_predict;
			world_model->curr_candidate_predict = world_model->large_candidate_predict;
			world_model->curr_candidate_iter = world_model->large_candidate_iter;

			world_model->num_states += NUM_STATE_CHANGE;

			world_model->large_obs_network = new StateNetwork(world_model->num_states + NUM_STATE_CHANGE, NUM_STATE_CHANGE);
			world_model->large_obs_network->resize();
			world_model->large_action_network = new StateNetwork(world_model->num_states + NUM_STATE_CHANGE, NUM_STATE_CHANGE);
			world_model->large_action_network->resize();

			world_model->large_final_network = new StateNetwork(world_model->curr_final_network);
			world_model->large_final_network->add_inputs(NUM_STATE_CHANGE);
			world_model->large_final_network->resize();

			world_model->large_epoch_iter = 0;
			world_model->large_average_max_update = 0.0;

			/**
			 * - don't need to change large_misguess_average and large_misguess_variance_average
			 */

			world_model->large_predict = new PredictWrapper(world_model->curr_predict);
			world_model->large_predict->add_states();

			world_model->large_candidate_predict = new PredictWrapper(world_model->curr_candidate_predict);
			world_model->large_candidate_predict->add_states();
			/**
			 * - simply don't change large_candidate_iter
			 */

			wrapper->solution->pad_new_state(NUM_STATE_CHANGE);

			world_model->curr_misguess_average_history.clear();
			world_model->curr_misguess_variance_average_history.clear();
			world_model->large_misguess_average_history.clear();
			world_model->large_misguess_variance_average_history.clear();
		}
	}
}
