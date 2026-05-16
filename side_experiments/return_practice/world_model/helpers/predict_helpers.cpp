#include "world_model_wrapper.h"

#include "network.h"
#include "world_model.h"
#include "world_model_wrapper.h"

using namespace std;

double predict_helper(vector<double>& existing_state,
					  vector<int>& potential_return,
					  WorldModelWrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

	vector<double> state = existing_state;
	for (int step_index = 0; step_index < (int)potential_return.size(); step_index++) {
		vector<double> starting_state = state;

		int action = potential_return[step_index];

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

	double sum_score = 0.0;
	for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
		vector<double> inputs;
		for (int i_index = 0; i_index < (int)world_model->final_network_inputs[n_index].size(); i_index++) {
			inputs.push_back(state[world_model->final_network_inputs[n_index][i_index]]);
		}
		world_model->final_networks[n_index]->activate(inputs);
		sum_score += world_model->final_networks[n_index]->output->acti_vals[0];
	}
	return sum_score;
}
