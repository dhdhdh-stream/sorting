#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "predict_wrapper.h"
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

void predict_helper(vector<double>& state,
					Wrapper* wrapper) {
	// WorldModel* world_model = wrapper->curr_model;

	// {
	// 	vector<double> obs;
	// 	uniform_int_distribution<int> obs_distribution(0, 4);
	// 	if (obs_distribution(generator) == 0) {
	// 		obs.push_back(1.0);
	// 	} else {
	// 		obs.push_back(0.0);
	// 	}
	// 	vector<double> inputs;
	// 	inputs.insert(inputs.end(), state.begin(), state.end());
	// 	inputs.insert(inputs.end(), obs.begin(), obs.end());
	// 	world_model->obs_network->activate(inputs);
	// 	for (int o_index = 0; o_index < world_model->num_states; o_index++) {
	// 		state[o_index] += world_model->obs_network->output->acti_vals[o_index];
	// 	}
	// }

	PredictWrapper* predict_wrapper = wrapper->curr_model->curr_predict;

	vector<double> select_vals(NUM_PREDICT);
	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		predict_wrapper->select_networks[n_index]->activate(state);
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
	uniform_real_distribution<double> select_distribution(0.0, sum_select);
	double select_rand_val = select_distribution(generator);
	int select_index;
	for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
		select_rand_val -= select_vals[n_index];
		if (select_rand_val <= 0.0) {
			select_index = n_index;
			break;
		}
	}

	predict_wrapper->val_networks[select_index]->activate(state);
	for (int o_index = 0; o_index < (int)state.size(); o_index++) {
		state[o_index] += predict_wrapper->val_networks[select_index]->output->acti_vals[o_index];
	}
}
