#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "state_network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

void view_helper(vector<vector<double>>& obs,
				 vector<int>& actions,
				 double target_val,
				 Wrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

	vector<double> state(world_model->num_states, 0.0);

	for (int step_index = 0; step_index < (int)obs.size(); step_index++) {
		{
			cout << "obs:";
			for (int o_index = 0; o_index < (int)obs[step_index].size(); o_index++) {
				cout << " " << obs[step_index][o_index];
			}
			cout << endl;

			vector<StateNetworkHistory*> step_obs_network_histories;
			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(state[world_model->network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), obs[step_index].begin(), obs[step_index].end());
				world_model->obs_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					state[world_model->network_outputs[n_index][o_index]] +=
						world_model->obs_networks[n_index]->output->acti_vals[o_index];
				}
			}

			cout << "state:";
			for (int s_index = 0; s_index < (int)state.size(); s_index++) {
				cout << " " << state[s_index];
			}
			cout << endl;
		}

		if (step_index < (int)actions.size()) {
			int action = actions[step_index];
			cout << "action: " << action << endl;

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
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(state[world_model->network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
				world_model->action_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					state[world_model->network_outputs[n_index][o_index]] +=
						world_model->action_networks[n_index]->output->acti_vals[o_index];
				}
			}

			cout << "state:";
			for (int s_index = 0; s_index < (int)state.size(); s_index++) {
				cout << " " << state[s_index];
			}
			cout << endl;
		}
	}

	world_model->final_network->activate(state);
	double predicted = world_model->final_network->output->acti_vals[0];
	cout << "predicted: " << predicted << endl;

	cout << "target_val: " << target_val << endl;
}

void view_world_model_helper(Wrapper* wrapper) {
	uniform_int_distribution<int> sample_distribution(0, wrapper->sample_obs.size()-1);
	int sample_index = sample_distribution(generator);
	view_helper(wrapper->sample_obs[sample_index],
				wrapper->sample_actions[sample_index],
				wrapper->sample_target_vals[sample_index],
				wrapper);
}
