#include "solution_node_if_start.h"

using namespace std;

void SolutionNodeIfStart::reset() override {
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		this->children_on[c_index] = false;
	}
}

SolutionNode* SolutionNodeIfStart::activate(Problem& problem,
											double* state_vals,
											bool* states_on,
											vector<SolutionNode*>& loop_scopes,
											vector<int>& loop_scope_counts,
											int visited_count,
											SolutionNode* explore_node,
											int& explore_type,
											double* potential_state_vals,
											bool* potential_states_on,
											std::vector<NetworkHistory*>& network_historys) override {
	vector<double> inputs;
	double curr_observations = problem.get_observation();
	inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			inputs.push_back(0.0);
		}
	}

	// always include 5% chance of selecting random

	int best_index = -1;
	double best_score = numeric_limits<double>::lowest();
	vector<NetworkHistory*> best_history;
	if (explore_type == EXPLORE_TYPE_STATE) {
		vector<int> potentials_on;
		vector<double> potential_vals;
		for (int p_index = 0; p_index < (int)this->network_inputs_potential_state_indexes.size(); p_index++) {
			if (potential_states_on[this->network_inputs_potential_state_indexes[p_index]]) {
				potentials_on.push_back(p_index);
				potential_vals.push_back(potential_state_vals[this->>network_inputs_potential_state_indexes[p_index]]);
			}
		}
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			vector<NetworkHistory*> temp_history;
			this->children_score_networks[c_index]->mtx.lock();
			this->children_score_networks[c_index]->activate(inputs,
															 potentials_on,
															 potential_vals,
															 temp_history);
			double child_score = this->children_score_networks[c_index]->output->acti_vals[0];
			this->children_score_networks[c_index]->mtx.unlock();

			if (child_score > best_score) {
				best_index = c_index;
				best_score = child_score;
				if (best_history.size() > 0) {
					delete best_history[0];
					best_history.pop_back();
				}
				best_history.push_back(temp_history[0]);
			} else {
				delete temp_history[0];
			}
		}
	} else {
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			vector<NetworkHistory*> temp_history;
			this->children_score_networks[c_index]->mtx.lock();
			this->children_score_networks[c_index]->activate(inputs, temp_history);
			double child_score = this->children_score_networks[c_index]->output->acti_vals[0];
			this->children_score_networks[c_index]->mtx.unlock();

			if (child_score > best_score) {
				best_index = c_index;
				best_score = child_score;
				if (best_history.size() > 0) {
					delete best_history[0];
					best_history.pop_back();
				}
				best_history.push_back(temp_history[0]);
			} else {
				delete temp_history[0];
			}
		}
	}

	if (visited_count == 0 && explore_node == NULL) {
		if (randuni() < 2.0/this->average_future_nodes) {
			if (rand()%2 == 0) {
				explore_node = this;
				explore_type = EXPLORE_TYPE_STATE;
			} else {
				explore_node = this;
				explore_type = EXPLORE_TYPE_PATH;
			}
		}
	}

	if (explore_node == this) {
		if (explore_type == EXPLORE_TYPE_STATE) {
			// explore state
		}
		else if (explore_type == EXPLORE_TYPE_PATH) {
			if (best_score < this->average) {
				// explore path
			}
		}
	}

	network_historys.push_back(best_history[0]);

	for (int o_index = 0; o_index < (int)this->scope_states_on.size(); o_index++) {
		state_vals[this->scope_states_on[o_index]] = 0.0;
		states_on[this->scope_states_on[o_index]] = true;
	}

	return this->children_nodes[best_index];
}

void SolutionNodeIfStart::backprop(double score,
								   SolutionNode* explore_node,
								   int& explore_type,
								   double* potential_state_errors,
								   bool* potential_states_on,
								   std::vector<NetworkHistory*>& network_historys) override {
	NetworkHistory* network_history = network_historys.back();

	int child_index;
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (network_history->network == this->children_score_networks[c_index]) {
			child_index = c_index;
			break;
		}
	}

	this->children_score_networks[child_index]->mtx.lock();

	network_history->reset_weights();

	vector<int> potentials_on = network_history->potentials_on;

	vector<double> errors;
	if (score == 1.0) {
		if (this->children_score_networks[child_index]->output->acti_vals[0] < 1.0) {
			errors.push_back(1.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			errors.push_back(0.0);
		}
	} else {
		if (this->children_score_networks[child_index]->output->acti_vals[0] > 0.0) {
			errors.push_back(0.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			errors.push_back(0.0);
		}
	}

	this->children_score_networks[child_index]->backprop(errors, potentials_on);

	for (int p_index = 0; p_index < (int)potentials_on.size(); p_index++) {
		potential_state_errors[this->network_inputs_potential_state_indexes[potentials_on[p_index]]] += \
			this->children_score_networks[child_index]->potential_inputs[potentials_on[o_index]]->errors[0];
		this->children_score_networks[child_index]->potential_inputs[potentials_on[o_index]]->errors[0] = 0.0;
	}

	this->children_score_networks[child_index]->mtx.unlock();
}
