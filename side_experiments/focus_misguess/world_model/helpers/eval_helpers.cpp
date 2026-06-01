#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

void eval_world_model_helper(vector<vector<double>>& obs,
							 vector<int>& actions,
							 double& predicted_score,
							 double& predicted_misguess,
							 Wrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

	uniform_int_distribution<int> include_obs_distribution(0, obs.size()-1);
	// int include_obs_index = include_obs_distribution(generator);
	int include_obs_index = obs.size()-1;

	vector<double> state(world_model->num_states, 0.0);

	for (int step_index = 0; step_index < (int)obs.size(); step_index++) {
		if (step_index <= include_obs_index) {
			vector<double> starting_state = state;

			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
					inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), obs[step_index].begin(),
					obs[step_index].end());
				world_model->obs_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
					state[world_model->obs_network_outputs[n_index][o_index]]
						+= world_model->obs_networks[n_index]->output->acti_vals[o_index];
				}
			}
		}

		if (step_index < (int)actions.size()) {
			int action = actions[step_index];

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

	double sum_misguess = 0.0;
	for (int n_index = 0; n_index < (int)world_model->misguess_networks.size(); n_index++) {
		vector<double> inputs;
		for (int i_index = 0; i_index < (int)world_model->misguess_network_inputs[n_index].size(); i_index++) {
			inputs.push_back(state[world_model->misguess_network_inputs[n_index][i_index]]);
		}
		world_model->misguess_networks[n_index]->activate(inputs);
		sum_misguess += world_model->misguess_networks[n_index]->output->acti_vals[0];
	}

	predicted_score = sum_score;
	predicted_misguess = sum_misguess;
}
