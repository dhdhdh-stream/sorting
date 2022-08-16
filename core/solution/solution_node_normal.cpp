#include "solution_node_normal.h"

using namespace std;

void SolutionNodeNormal::reset() override {
	// do nothing
}

SolutionNode* SolutionNodeNormal::tune(Problem& problem,
									   double* state_vals,
									   bool* states_on,
									   vector<NetworkHistory*>& network_historys) override {
	for (int sn_index = 0; sn_index < (int)this->state_networks_target_states.size(); sn_index++) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			vector<double> state_network_inputs;
			double curr_observations = problem.get_observation();
			state_network_inputs.push_back(curr_observations);
			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[i_index]]) {
					state_network_inputs.push_back(state_vals[this->state_network_inputs_state_indexes[i_index]]);
				} else {
					state_network_inputs.push_back(0.0);
				}
			}
			this->state_networks[sn_index]->mtx.lock();
			this->state_networks[sn_index]->activate(state_network_inputs, network_historys);
			state_vals[this->state_networks_target_states[sn_index]] = \
				this->state_networks[sn_index]->val_val->acti_vals[0];
			this->state_networks[sn_index]->mtx.unlock();
		}
	}

	problem.perform_action(this->action);

	tune_score_network(problem,
					   state_vals,
					   states_on,
					   network_historys);

	return this->next;
}

void SolutionNodeNormal::tune_update(double score,
									 double* state_errors,
									 bool* states_on,
									 std::vector<NetworkHistory*>& network_historys) override {
	tune_update_score_network(score,
							  state_errors,
							  states_on,
							  network_historys);

	for (int sn_index = (int)this->state_networks_target_states.size()-1; sn_index >= 0; sn_index--) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			vector<double> state_network_errors;
			state_network_errors.push_back(state_errors[this->state_networks_target_states[sn_index]]);
			this->state_networks[sn_index]->mtx.lock();
			this->state_networks[sn_index]->backprop(state_network_errors);
			for (int bi_index = 0; bi_index < (int)this->state_networks_backprop_indexes[sn_index].size(); bi_index++) {
				int state_index = this->state_network_inputs_state_indexes[sn_index][bi_index];
				state_errors[state_index] = this->state_networks[sn_index]->input->errors[bi_index];
				this->state_networks[sn_index]->input->errors[bi_index] = 0.0;
			}
			this->state_networks[sn_index]->mtx.unlock();
		}
	}
}
