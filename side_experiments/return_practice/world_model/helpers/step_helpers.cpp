#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "state_network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

void obs_helper(vector<double>& obs,
				vector<double>& state,
				Wrapper* wrapper) {
	WorldModel* world_model = wrapper->curr_model;

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
				   Wrapper* wrapper) {
	WorldModel* world_model = wrapper->curr_model;

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
	world_model->action_network->activate(inputs);
	for (int o_index = 0; o_index < world_model->num_states; o_index++) {
		state[o_index] += world_model->action_network->output->acti_vals[o_index];
	}
}

void predict_helper(int action,
					vector<double>& state,
					Wrapper* wrapper) {
	WorldModel* world_model = wrapper->curr_model;

	// vector<double> partial_inputs;
	// for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
	// 	if (action == a_index) {
	// 		partial_inputs.push_back(1.0);
	// 	} else {
	// 		partial_inputs.push_back(0.0);
	// 	}
	// }

	// vector<double> inputs;
	// inputs.insert(inputs.end(), state.begin(), state.end());
	// inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
	// world_model->predict_network->activate(inputs);
	// for (int o_index = 0; o_index < world_model->num_states; o_index++) {
	// 	state[o_index] += world_model->predict_network->output->acti_vals[o_index];
	// }

	{
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
		world_model->action_network->activate(inputs);
		for (int o_index = 0; o_index < world_model->num_states; o_index++) {
			state[o_index] += world_model->action_network->output->acti_vals[o_index];
		}
	}

	{
		vector<double> obs;
		uniform_int_distribution<int> obs_distribution(0, 4);
		if (obs_distribution(generator) == 0) {
			obs.push_back(1.0);
		} else {
			obs.push_back(0.0);
		}
		vector<double> inputs;
		inputs.insert(inputs.end(), state.begin(), state.end());
		inputs.insert(inputs.end(), obs.begin(), obs.end());
		world_model->obs_network->activate(inputs);
		for (int o_index = 0; o_index < world_model->num_states; o_index++) {
			state[o_index] += world_model->obs_network->output->acti_vals[o_index];
		}
	}
}
