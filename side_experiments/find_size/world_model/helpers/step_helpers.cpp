#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"

using namespace std;

void obs_helper(vector<double>& obs,
				vector<double>& state,
				WorldModel* world_model) {
	vector<double> inputs;
	inputs.insert(inputs.end(), state.begin(), state.end());
	inputs.insert(inputs.end(), obs.begin(), obs.end());
	world_model->obs_network->activate(inputs);
	for (int o_index = 0; o_index < world_model->num_states; o_index++) {
		state[o_index] += world_model->obs_network->output->acti_vals[o_index];
	}
}

void action_helper(int action,
				   vector<double>& state,
				   WorldModel* world_model) {
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
	world_model->action_network->activate(inputs);
	for (int o_index = 0; o_index < world_model->num_states; o_index++) {
		state[o_index] += world_model->action_network->output->acti_vals[o_index];
	}
}
