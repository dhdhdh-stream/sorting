#include "world_model_helpers.h"

#include <iostream>

#include "constants.h"
#include "experiment_run.h"
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
const int UPDATE_MIN_SAMPLE_SIZE = 100;
// const int ITERS_PER_UPDATE = 100;
const int ITERS_PER_UPDATE = 1;
#endif /* MDEBUG */

void predict_update_helper(vector<double>& start_state,
						   vector<double>& end_state,
						   PredictWrapper* predict_wrapper,
						   Wrapper* wrapper) {
	vector<double> diff(start_state.size());
	for (int s_index = 0; s_index < (int)start_state.size(); s_index++) {
		diff[s_index] = end_state[s_index] - start_state[s_index];
	}

	// // temp
	// if ((wrapper->world_model->curr_candidate_iter+1) % 500000 == 0) {
	// 	cout << "diff:";
	// 	for (int d_index = 0; d_index < (int)diff.size(); d_index++) {
	// 		cout << " " << diff[d_index];
	// 	}
	// 	cout << endl;
	// }

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
			// // temp
			// if ((wrapper->world_model->curr_candidate_iter+1) % 500000 == 0) {
			// 	cout << "predict_wrapper->val_networks[min_index]->output->acti_vals:";
			// 	for (int d_index = 0; d_index < (int)predict_wrapper->val_networks[min_index]->output->acti_vals.size(); d_index++) {
			// 		cout << " " << predict_wrapper->val_networks[min_index]->output->acti_vals[d_index];
			// 	}
			// 	cout << endl;
			// 	cout << "min_index: " << min_index << endl;
			// }

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

		// // temp
		// if ((wrapper->world_model->curr_candidate_iter+1) % 500000 == 0) {
		// 	cout << "select_vals:";
		// 	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		// 		cout << " " << select_vals[n_index];
		// 	}
		// 	cout << endl;
		// }

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

	vector<vector<StateNetworkHistory*>> obs_network_histories;
	vector<vector<StateNetworkHistory*>> action_network_histories;

	for (int step_index = 0; step_index < (int)obs.size(); step_index++) {
		{
			vector<double> start_state = state;

			vector<StateNetworkHistory*> step_obs_network_histories;
			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(state[world_model->network_inputs[n_index][i_index]]);
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

			world_model->candidate_iter++;
			if ((world_model->candidate_iter+1) % ITERS_PER_PREDICT == 0) {
				predict_update_helper(start_state,
									  state,
									  world_model->predict,
									  wrapper);
				predict_update_helper(start_state,
									  state,
									  world_model->candidate_predict,
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

			vector<StateNetworkHistory*> step_action_network_histories;
			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(state[world_model->network_inputs[n_index][i_index]]);
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
		}
	}

	world_model->final_network->activate(state);
	double predicted = world_model->final_network->output->acti_vals[0];

	vector<double> final_errors{target_val - predicted};

	vector<double> state_errors(world_model->num_states, 0.0);

	world_model->final_network->backprop(final_errors);
	for (int i_index = 0; i_index < world_model->num_states; i_index++) {
		state_errors[i_index] += world_model->final_network->input->errors[i_index];
		world_model->final_network->input->errors[i_index] = 0.0;
	}

	for (int step_index = (int)obs.size()-1; step_index >= 0; step_index--) {
		if (step_index < (int)actions.size()) {
			for (int n_index = (int)world_model->action_networks.size()-1; n_index >= 0; n_index--) {
				vector<double> errors;
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					errors.push_back(state_errors[world_model->network_outputs[n_index][o_index]]);
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
		}

		{
			for (int n_index = (int)world_model->obs_networks.size()-1; n_index >= 0; n_index--) {
				vector<double> errors;
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					errors.push_back(state_errors[world_model->network_outputs[n_index][o_index]]);
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
		}
	}

	world_model->epoch_iter++;
	if (world_model->epoch_iter >= NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
			world_model->obs_networks[n_index]->get_max_update(max_update);
		}
		for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
			world_model->action_networks[n_index]->get_max_update(max_update);
		}
		world_model->final_network->get_max_update(max_update);

		world_model->average_max_update = 0.999*world_model->average_max_update + 0.001*max_update;

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
			world_model->final_network->update_weights(learning_rate);
		}

		world_model->epoch_iter = 0;
	}

	if (world_model->candidate_iter >= PREDICT_CANDIDATE_CHECK_NUM_ITERS) {
		if (world_model->candidate_predict->misguess_average < world_model->predict->misguess_average) {
			delete world_model->predict;
			world_model->predict = world_model->candidate_predict;
		} else {
			delete world_model->candidate_predict;
		}

		world_model->candidate_predict = new PredictWrapper(world_model->predict);
		world_model->candidate_predict->twiddle();

		world_model->candidate_iter = 0;
	}
}

void update_helper(double target_val,
				   ExperimentRun* run) {
	WorldModel* world_model = run->wrapper->world_model;

	vector<double> state_errors(world_model->num_states, 0.0);

	{
		world_model->final_network->activate(run->state);
		double predicted = world_model->final_network->output->acti_vals[0];

		vector<double> final_errors{target_val - predicted};

		world_model->final_network->backprop(final_errors);
		for (int i_index = 0; i_index < world_model->num_states; i_index++) {
			state_errors[i_index] += world_model->final_network->input->errors[i_index];
			world_model->final_network->input->errors[i_index] = 0.0;
		}
	}

	for (int step_index = (int)run->obs_histories.size()-1; step_index >= 0; step_index--) {
		if (step_index < (int)run->action_histories.size()) {
			for (int n_index = (int)world_model->action_networks.size()-1; n_index >= 0; n_index--) {
				vector<double> errors;
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					errors.push_back(state_errors[world_model->network_outputs[n_index][o_index]]);
				}
				world_model->action_networks[n_index]->backprop(errors,
																run->action_network_histories[step_index][n_index]);
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					state_errors[world_model->network_inputs[n_index][i_index]] +=
						world_model->action_networks[n_index]->input->errors[i_index];
					world_model->action_networks[n_index]->input->errors[i_index] = 0.0;
				}
			}
		}

		for (int n_index = (int)run->taken_branch_node_networks[step_index].size()-1; n_index >= 0; n_index--) {
			double error = target_val - run->taken_branch_node_networks[step_index][n_index]->output->acti_vals[0];
			run->taken_branch_node_networks[step_index][n_index]->backprop(error);
			for (int i_index = 0; i_index < world_model->num_states; i_index++) {
				state_errors[i_index] += run->taken_branch_node_networks[step_index][n_index]->input->errors[i_index];
				run->taken_branch_node_networks[step_index][n_index]->input->errors[i_index] = 0.0;
			}
		}

		{
			for (int n_index = (int)world_model->obs_networks.size()-1; n_index >= 0; n_index--) {
				vector<double> errors;
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					errors.push_back(state_errors[world_model->network_outputs[n_index][o_index]]);
				}
				world_model->obs_networks[n_index]->backprop(errors,
															 run->obs_network_histories[step_index][n_index]);
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					state_errors[world_model->network_inputs[n_index][i_index]] +=
						world_model->obs_networks[n_index]->input->errors[i_index];
					world_model->obs_networks[n_index]->input->errors[i_index] = 0.0;
				}
			}
		}
	}

	world_model->epoch_iter++;
	if (world_model->epoch_iter >= NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
			world_model->obs_networks[n_index]->get_max_update(max_update);
		}
		for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
			world_model->action_networks[n_index]->get_max_update(max_update);
		}
		world_model->final_network->get_max_update(max_update);

		world_model->average_max_update = 0.999*world_model->average_max_update + 0.001*max_update;

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
			world_model->final_network->update_weights(learning_rate);
		}

		world_model->epoch_iter = 0;
	}

	if (world_model->candidate_iter >= PREDICT_CANDIDATE_CHECK_NUM_ITERS) {
		if (world_model->candidate_predict->misguess_average < world_model->predict->misguess_average) {
			delete world_model->predict;
			world_model->predict = world_model->candidate_predict;
		} else {
			delete world_model->candidate_predict;
		}

		world_model->candidate_predict = new PredictWrapper(world_model->predict);
		world_model->candidate_predict->twiddle();

		world_model->candidate_iter = 0;
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
