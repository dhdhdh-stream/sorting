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
	WorldModel* world_model = wrapper->world_model;

	vector<double> start_state = state;

	for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
		vector<double> inputs;
		for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
			inputs.push_back(start_state[world_model->network_inputs[n_index][i_index]]);
		}
		inputs.insert(inputs.end(), obs.begin(), obs.end());
		world_model->obs_networks[n_index]->activate(inputs);
		for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
			state[world_model->network_outputs[n_index][o_index]] += world_model->obs_networks[n_index]->output->acti_vals[o_index];
		}
	}
}

void action_helper(int action,
				   vector<double>& state,
				   Wrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

	vector<double> start_state = state;

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
			inputs.push_back(start_state[world_model->network_inputs[n_index][i_index]]);
		}
		inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
		world_model->action_networks[n_index]->activate(inputs);
		for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
			state[world_model->network_outputs[n_index][o_index]] += world_model->action_networks[n_index]->output->acti_vals[o_index];
		}
	}
}

void predict_helper(vector<double>& state,
					Wrapper* wrapper) {
	PredictWrapper* predict_wrapper = wrapper->world_model->curr_predict;

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

	// // temp
	// cout << "select_vals:" << endl;
	// for (int n_index = 0; n_index < NUM_PREDICT; n_index++) {
	// 	cout << n_index << ": " << select_vals[n_index] << endl;
	// }

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

	// // temp
	// cout << "select_index: " << select_index << endl;

	predict_wrapper->val_networks[select_index]->activate(state);
	for (int o_index = 0; o_index < (int)state.size(); o_index++) {
		state[o_index] += predict_wrapper->val_networks[select_index]->output->acti_vals[o_index];
	}
}
