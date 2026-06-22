#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment_run.h"
#include "globals.h"
#include "network.h"
#include "solution.h"
#include "start_node.h"
#include "state_network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int UPDATE_MIN_NUM_SAMPLES = 10;
const int UPDATE_ITERS = 2;
#else
const int UPDATE_MIN_NUM_SAMPLES = 100;
// const int UPDATE_ITERS = 100;
const int UPDATE_ITERS = 1;
#endif /* MDEBUG */

const int RAMP_UPDATE_MIN_SAMPLES = 10;
#if defined(MDEBUG) && MDEBUG
const int RAMP_UPDATE_NUM_TRAIN = 2;
const int ITERS_PER_RAMP = 2;
#else
const int RAMP_UPDATE_NUM_TRAIN = 100;
const int ITERS_PER_RAMP = 100;
#endif /* MDEBUG */

void update_helper(vector<vector<double>>& obs,
				   vector<int>& actions,
				   double target_val,
				   Network* network,
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

	network->activate(state);
	double predicted = network->output->acti_vals[0];

	double final_error = target_val - predicted;

	vector<double> state_errors(world_model->num_states, 0.0);

	network->backprop(final_error);
	for (int i_index = 0; i_index < world_model->num_states; i_index++) {
		state_errors[i_index] += network->input->errors[i_index];
		network->input->errors[i_index] = 0.0;
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
}

void update_solution_helper(ExperimentRun* run,
							double target_val,
							Wrapper* wrapper) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = run->node_histories.begin();
			h_it != run->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_START:
			{
				StartNodeHistory* start_node_history = (StartNodeHistory*)h_it->second;
				StartNode* start_node = (StartNode*)start_node_history->node;

				start_node->curr_instances_per_run++;

				if (start_node->state_history.size() < STATE_NUM_SAVE) {
					start_node->state_history.push_back(start_node_history->state);
				} else {
					start_node->state_history[start_node->history_index] = start_node_history->state;
				}
				start_node->history_index++;
				if (start_node->history_index >= STATE_NUM_SAVE) {
					start_node->history_index = 0;
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)h_it->second;
				ActionNode* action_node = (ActionNode*)action_node_history->node;

				action_node->curr_instances_per_run++;

				if (action_node->state_history.size() < STATE_NUM_SAVE) {
					action_node->state_history.push_back(action_node_history->state);
				} else {
					action_node->state_history[action_node->history_index] = action_node_history->state;
				}
				action_node->history_index++;
				if (action_node->history_index >= STATE_NUM_SAVE) {
					action_node->history_index = 0;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					branch_node->branch_curr_instances_per_run++;

					if (branch_node->branch_state_history.size() < STATE_NUM_SAVE) {
						branch_node->branch_state_history.push_back(branch_node_history->state);
						branch_node->branch_target_val_history.push_back(target_val);
					} else {
						branch_node->branch_state_history[branch_node->branch_history_index] = branch_node_history->state;
						branch_node->branch_target_val_history[branch_node->branch_history_index] = target_val;
					}
					branch_node->branch_history_index++;
					if (branch_node->branch_history_index >= STATE_NUM_SAVE) {
						branch_node->branch_history_index = 0;
					}

					if (branch_node->ramp >= RAMP_NUM_GEARS) {
						// if (branch_node->branch_state_history.size() >= UPDATE_MIN_NUM_SAMPLES) {
						// 	uniform_int_distribution<int> distribution(0, branch_node->branch_state_history.size()-1);
						// 	for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
						// 		int index = distribution(generator);
						// 		branch_node->branch_network->activate(branch_node->branch_state_history[index]);
						// 		double error = branch_node->branch_target_val_history[index] - branch_node->branch_network->output->acti_vals[0];
						// 		branch_node->branch_network->backprop(error);
						// 	}
						// }
					} else {
						// if (branch_node->branch_state_history.size() >= RAMP_UPDATE_MIN_SAMPLES) {
						// 	uniform_int_distribution<int> distribution(0, branch_node->branch_state_history.size()-1);
						// 	for (int iter_index = 0; iter_index < RAMP_UPDATE_NUM_TRAIN; iter_index++) {
						// 		int index = distribution(generator);
						// 		branch_node->branch_network->activate(branch_node->branch_state_history[index]);
						// 		double error = branch_node->branch_target_val_history[index] - branch_node->branch_network->output->acti_vals[0];
						// 		branch_node->branch_network->backprop(error);
						// 	}
						// }

						// TODO: to give added weight, can try simply multiplying error signal
						vector<vector<double>> obs(run->obs_histories.begin(), run->obs_histories.begin() + branch_node_history->obs_history_index);
						vector<int> actions(run->action_histories.begin(), run->action_histories.begin() + branch_node_history->obs_history_index-1);
						update_helper(obs,
									  actions,
									  target_val,
									  branch_node->branch_network,
									  wrapper);

						branch_node->ramp_iter++;
						if (branch_node->ramp_iter >= ITERS_PER_RAMP) {
							branch_node->ramp++;
							branch_node->ramp_iter = 0;

							// // temp
							// cout << "branch_node->ramp: " << branch_node->ramp << endl;

							if (branch_node->ramp >= RAMP_NUM_GEARS) {
								wrapper->solution->timestamp++;
							}
						}
					}

					// vector<vector<double>> obs(run->obs_histories.begin(), run->obs_histories.begin() + branch_node_history->obs_history_index);
					// vector<int> actions(run->action_histories.begin(), run->action_histories.begin() + branch_node_history->obs_history_index-1);
					// update_helper(obs,
					// 			  actions,
					// 			  target_val,
					// 			  branch_node->branch_network,
					// 			  wrapper);
				} else {
					branch_node->original_curr_instances_per_run++;

					if (branch_node->original_state_history.size() < STATE_NUM_SAVE) {
						branch_node->original_state_history.push_back(branch_node_history->state);
						branch_node->original_target_val_history.push_back(target_val);
					} else {
						branch_node->original_state_history[branch_node->original_history_index] = branch_node_history->state;
						branch_node->original_target_val_history[branch_node->original_history_index] = target_val;
					}
					branch_node->original_history_index++;
					if (branch_node->original_history_index >= STATE_NUM_SAVE) {
						branch_node->original_history_index = 0;
					}

					if (branch_node->ramp >= RAMP_NUM_GEARS) {
						// if (branch_node->original_state_history.size() >= UPDATE_MIN_NUM_SAMPLES) {
						// 	uniform_int_distribution<int> distribution(0, branch_node->original_state_history.size()-1);
						// 	for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
						// 		int index = distribution(generator);
						// 		branch_node->original_network->activate(branch_node->original_state_history[index]);
						// 		double errors = branch_node->original_target_val_history[index] - branch_node->original_network->output->acti_vals[0];
						// 		branch_node->original_network->backprop(errors);
						// 	}
						// }
					} else {
						// if (branch_node->original_state_history.size() >= RAMP_UPDATE_MIN_SAMPLES) {
						// 	uniform_int_distribution<int> distribution(0, branch_node->original_state_history.size()-1);
						// 	for (int iter_index = 0; iter_index < RAMP_UPDATE_NUM_TRAIN; iter_index++) {
						// 		int index = distribution(generator);
						// 		branch_node->original_network->activate(branch_node->original_state_history[index]);
						// 		double error = branch_node->original_target_val_history[index] - branch_node->original_network->output->acti_vals[0];
						// 		branch_node->original_network->backprop(error);
						// 	}
						// }

						vector<vector<double>> obs(run->obs_histories.begin(), run->obs_histories.begin() + branch_node_history->obs_history_index);
						vector<int> actions(run->action_histories.begin(), run->action_histories.begin() + branch_node_history->obs_history_index-1);
						update_helper(obs,
									  actions,
									  target_val,
									  branch_node->original_network,
									  wrapper);

						branch_node->original_iter++;
					}

					// vector<vector<double>> obs(run->obs_histories.begin(), run->obs_histories.begin() + branch_node_history->obs_history_index);
					// vector<int> actions(run->action_histories.begin(), run->action_histories.begin() + branch_node_history->obs_history_index-1);
					// update_helper(obs,
					// 			  actions,
					// 			  target_val,
					// 			  branch_node->original_network,
					// 			  wrapper);
				}
			}
			break;
		}
	}

	for (map<int, AbstractNode*>::iterator it = wrapper->solution->nodes.begin();
			it != wrapper->solution->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)it->second;
				start_node->average_instances_per_run = 0.999*start_node->average_instances_per_run
					+ 0.001*start_node->curr_instances_per_run;
				start_node->curr_instances_per_run = 0;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;
				action_node->average_instances_per_run = 0.999*action_node->average_instances_per_run
					+ 0.001*action_node->curr_instances_per_run;
				action_node->curr_instances_per_run = 0;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				branch_node->original_average_instances_per_run = 0.999*branch_node->original_average_instances_per_run
					+ 0.001*branch_node->original_curr_instances_per_run;
				branch_node->original_curr_instances_per_run = 0;
				branch_node->branch_average_instances_per_run = 0.999*branch_node->branch_average_instances_per_run
					+ 0.001*branch_node->branch_curr_instances_per_run;
				branch_node->branch_curr_instances_per_run = 0;
			}
			break;
		}
	}
}
