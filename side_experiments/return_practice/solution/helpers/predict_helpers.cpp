#include "solution_helpers.h"

#include "abstract_node.h"
#include "predict_run.h"
#include "state_network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

void calc_state_helper(vector<vector<double>>& obs,
					   vector<int>& actions,
					   Wrapper* wrapper,
					   vector<double>& state) {
	WorldModel* world_model = wrapper->world_model;

	state = vector<double>(world_model->num_states, 0.0);

	for (int step_index = 0; step_index < (int)obs.size(); step_index++) {
		{
			vector<double> start_state = state;

			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(start_state[world_model->network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), obs[step_index].begin(), obs[step_index].end());
				world_model->obs_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					state[world_model->network_outputs[n_index][o_index]] +=
						world_model->obs_networks[n_index]->output->acti_vals[o_index];
				}
			}
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

			vector<double> start_state = state;

			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(start_state[world_model->network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
				world_model->action_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					state[world_model->network_outputs[n_index][o_index]] +=
						world_model->action_networks[n_index]->output->acti_vals[o_index];
				}
			}
		}
	}
}

double predict_helper(vector<double>& state,
					  AbstractNode* next_node,
					  Wrapper* wrapper) {
	PredictRun* run = new PredictRun();

	run->wrapper = wrapper;

	run->node_context = next_node;

	run->state = state;

	while (run->node_context != NULL) {
		run->node_context->predict_step(run);
	}

	wrapper->world_model->final_network->activate(run->state);
	double predicted = wrapper->world_model->final_network->output->acti_vals[0];

	delete run;

	return predicted;
}
