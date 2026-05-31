#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

void obs_helper(vector<double>& obs,
				vector<double>& state,
				Wrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

	vector<double> starting_state = state;

	for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
		vector<double> inputs;
		for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
			inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
		}
		inputs.insert(inputs.end(), obs.begin(), obs.end());
		world_model->obs_networks[n_index]->activate(inputs);
		for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
			state[world_model->obs_network_outputs[n_index][o_index]]
				+= world_model->obs_networks[n_index]->output->acti_vals[o_index];
		}
	}
}

void action_helper(int action,
				   vector<double>& state,
				   Wrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

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
