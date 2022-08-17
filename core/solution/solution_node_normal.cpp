#include "solution_node_normal.h"

using namespace std;

void SolutionNodeNormal::reset() override {
	// do nothing
}

SolutionNode* SolutionNodeNormal::activate(Problem& problem,
										   double* state_vals,
										   bool* states_on,
										   int visited_count,
										   SolutionNode* explore_node,
										   int& explore_type,
										   double* potential_state_vals,
										   bool* potential_states_on,
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
			this->state_networks[sn_index]->activate(state_network_inputs);
			state_vals[this->state_networks_target_states[sn_index]] = \
				this->state_networks[sn_index]->output->acti_vals[0];
			this->state_networks[sn_index]->mtx.unlock();
		}
	}

	if (explore_type == EXPLORE_TYPE_STATE) {
		for (int p_index = 0; p_index < (int)this->potential_state_networks_target_states.size(); p_index++) {
			if (potential_states_on[this->potential_state_networks_target_states[p_index]]) {
				vector<double> state_network_inputs;
				double curr_observations = problem.get_observation();
				state_network_inputs.push_back(curr_observations);
				for (int i_index = 0; i_index < (int)this->potential_inputs_state_indexes[sn_index].size(); i_index++) {
					if (states_on[this->potential_inputs_state_indexes[i_index]]) {
						state_network_inputs.push_back(state_vals[this->potential_inputs_state_indexes[i_index]]);
					} else {
						state_network_inputs.push_back(0.0);
					}
				}
				for (int pi_index = 0; pi_index < (int)this->potential_potential_inputs_state_indexes.size(); pi_index++) {
					if (potential_states_on[this->potential_potential_inputs_state_indexes[pi_index]]) {
						state_network_inputs.push_back(potential_state_vals[this->potential_potential_inputs_state_indexes[pi_index]]);
					} else {
						state_network_inputs.push_back(0.0);
					}
				}
				this->potential_state_networks[p_index]->mtx.lock();
				this->potential_state_networks[p_index]->activate(state_network_inputs, network_historys);
				potential_state_vals[this->potential_state_networks_target_states[p_index]] = \
					this->potential_state_vals[p_index]->output->acti_vals[0];
				this->potential_state_networks[p_index]->mtx.unlock();
			}
		}
	}

	problem.perform_action(this->action);

	double score = activate_score_network(problem,
										  state_vals,
										  states_on,
										  explore_type,
										  potential_state_vals,
										  potential_states_on,
										  network_historys);

	if (visited_count == 0 && explore_node == NULL) {
		if (randuni() < (1.0/this->average_future_nodes)) {
			explore_node = this;
			explore_type = EXPLORE_TYPE_PATH;
		}
	}

	if (explore_node == this) {
		if (score < this->average) {
			// explore path
		}
	}

	return this->next;
}

void SolutionNodeNormal::backprop(double score,
								  SolutionNode* explore_node,
								  int& explore_type,
								  double* potential_state_errors,
								  bool* potential_states_on,
								  std::vector<NetworkHistory*>& network_historys) override {
	backprop_score_network(score,
						   potential_state_errors,
						   network_historys);

	if (explore_type == EXPLORE_TYPE_STATE) {
		for (int p_index = (int)this->potential_state_networks_target_states.size()-1; p_index >= 0; p_index--) {
			if (potential_states_on[this->potential_state_networks_target_states[p_index]]) {
				this->potential_state_networks[p_index]->mtx.lock();

				NetworkHistory* network_history = network_historys.back();
				network_history->reset_weights();

				vector<double> state_network_errors;
				state_network_errors.push_back(potential_state_errors[
					this->potential_state_networks_target_states[p_index]]);
				this->potential_state_networks[p_index]->backprop(state_network_errors);

				for (int pi_index = 0; pi_index < (int)this->potential_potential_inputs_state_indexes.size(); pi_index++) {
					if (potential_states_on[this->potential_potential_inputs_state_indexes[pi_index]]) {
						if (this->potential_potential_inputs_state_indexes[pi_index]
								== this->potential_state_networks_target_states[p_index]) {
							potential_state_errors[this->potential_potential_inputs_state_indexes[pi_index]] = \
								this->potential_state_networks[p_index]->input->errors[
									this->potential_inputs_state_indexes.size() + pi_index];
						} else {
							potential_state_errors[this->potential_potential_inputs_state_indexes[pi_index]] += \
								this->potential_state_networks[p_index]->input->errors[
									this->potential_inputs_state_indexes.size() + pi_index];
						}
					}

					this->potential_state_networks[p_index]->input->errors[
						this->potential_inputs_state_indexes.size() + pi_index] = 0.0;
				}

				this->potential_state_networks[p_index]->mtx.unlock();

				delete network_history;
				network_historys.pop_back();
			}
		}
	}
}

void SolutionNodeNormal::increment(SolutionNode* explore_node,
								   int& explore_type,
								   bool* potential_states_on) override {
	increment_score_network();

	if (explore_type == EXPLORE_TYPE_STATE) {
		for (int p_index = (int)this->potential_state_networks_target_states.size()-1; p_index >= 0; p_index--) {
			if (potential_states_on[this->potential_state_networks_target_states[p_index]]) {
				this->potential_state_networks[p_index]->mtx.lock();
				this->potential_state_networks[p_index]->increment();
				this->potential_state_networks[p_index]->mtx.unlock();
			}
		}
	}
}
