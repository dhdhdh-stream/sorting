#include "world_model_helpers.h"

#include "globals.h"
#include "network.h"
#include "problem.h"
#include "world_model.h"
#include "world_model_wrapper.h"

using namespace std;

const int NUM_PREDICTS_PER_STEP = 50;

void explore_obs_step(std::vector<double>& obs,
					  Run& run,
					  WorldModelWrapper* wrapper) {
	run.obs_histories.push_back(obs);

	WorldModel* world_model = wrapper->world_model;

	vector<double> starting_state = run.state;

	for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
		vector<double> inputs = obs;
		for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
			inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
		}
		world_model->obs_networks[n_index]->activate(inputs);
		for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
			run.state[world_model->obs_network_outputs[n_index][o_index]]
				+= world_model->obs_networks[n_index]->output->acti_vals[o_index];
		}
	}
}

void explore_action_step(Run& run,
						 int& next_action,
						 bool& is_done,
						 WorldModelWrapper* wrapper) {
	if (run.commit_index < (int)run.commit.size()) {
		next_action = run.commit[run.commit_index];
		run.commit_index++;
	} else {
		vector<int> best_return = run.curr_return;
		double best_predicted = predict_helper(run.state,
											   run.curr_return,
											   wrapper);

		geometric_distribution<int> num_actions_distribution(0.1);
		uniform_int_distribution<int> action_distribution(0, wrapper->num_actions-1);
		for (int try_index = 0; try_index < NUM_PREDICTS_PER_STEP; try_index++) {
			int num_actions = num_actions_distribution(generator);
			vector<int> potential_return;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				potential_return.push_back(action_distribution(generator));
			}
			double potential_predicted = predict_helper(run.state,
														potential_return,
														wrapper);
			if (potential_predicted > best_predicted) {
				best_return = potential_return;
				best_predicted = potential_predicted;
			}
		}

		if (best_return.size() == 0) {
			is_done = true;
			return;
		}

		next_action = best_return[0];
		best_return.erase(best_return.begin());
		run.curr_return = best_return;
	}

	run.action_histories.push_back(next_action);

	WorldModel* world_model = wrapper->world_model;

	vector<double> starting_state = run.state;

	vector<double> partial_inputs;
	for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
		if (next_action == a_index) {
			partial_inputs.push_back(1.0);
		} else {
			partial_inputs.push_back(0.0);
		}
	}

	for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
		vector<double> inputs = partial_inputs;
		for (int i_index = 0; i_index < (int)world_model->action_network_inputs[n_index].size(); i_index++) {
			inputs.push_back(starting_state[world_model->action_network_inputs[n_index][i_index]]);
		}
		world_model->action_networks[n_index]->activate(inputs);
		for (int o_index = 0; o_index < (int)world_model->action_network_outputs[n_index].size(); o_index++) {
			run.state[world_model->action_network_outputs[n_index][o_index]]
				+= world_model->action_networks[n_index]->output->acti_vals[o_index];
		}
	}
}
