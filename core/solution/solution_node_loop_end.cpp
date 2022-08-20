#include "solution_node_loop_end.h"

using namespace std;

void SolutionNodeLoopEnd::reset() override {
	// do nothing
}

void SolutionNodeLoopEnd::add_potential_state(vector<int> potential_state_indexes,
											  SolutionNode* scope) override {
	this->no_halt->add_potential_state(potential_state_indexes, scope);

	if (this->start == scope) {
		return;
	}

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		this->network_inputs_potential_state_indexes.push_back(
			potential_state_indexes[ps_index]);

		this->halt_network->add_potential();
		this->no_halt_network->add_potential();
	}

	if (this->halt->type == NODE_TYPE_IF_END) {
		return;
	} else if (this->halt->type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* halt_loop_start = (SolutionNodeLoopStart*)this->halt;
		if (halt_loop_start->loop_in == this) {
			return;
		}
	}
	this->halt->add_potential_state(potential_state_indexes, scope);
}

void SolutionNodeLoopEnd::extend_with_potential_state(vector<int> potential_state_indexes,
													  vector<int> new_state_indexes,
													  SolutionNode* scope) override {
	this->no_halt->extend_with_potential_state(potential_state_indexes,
											   new_state_indexes,
											   scope);

	if (this->start == scope) {
		return;
	}

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->network_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->network_inputs_state_indexes.push_back(new_state_indexes[ps_index]);
				
				this->halt_network->extend_with_potential(pi_index);
				this->no_halt_network->extend_with_potential(pi_index);

				break;
			}
		}
	}

	if (this->halt->type == NODE_TYPE_IF_END) {
		return;
	} else if (this->halt->type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* halt_loop_start = (SolutionNodeLoopStart*)this->halt;
		if (halt_loop_start->loop_in == this) {
			return;
		}
	}
	this->halt->extend_with_potential_state(potential_state_indexes,
											new_state_indexes,
											scope);
}

void SolutionNodeLoopEnd::reset_potential_state(vector<int> potential_state_indexes,
												SolutionNode* scope) override {
	this->no_halt->reset_potential_state(potential_state_indexes, scope);

	if (this->start == scope) {
		return;
	}

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->network_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->halt_network->reset_potential(pi_index);
				this->no_halt_network->reset_potential(pi_index);
			}
		}
	}

	if (this->halt->type == NODE_TYPE_IF_END) {
		return;
	} else if (this->halt->type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* halt_loop_start = (SolutionNodeLoopStart*)this->halt;
		if (halt_loop_start->loop_in == this) {
			return;
		}
	}
	this->halt->reset_potential_state(potential_state_indexes, scope);
}

void SolutionNodeLoopEnd::clear_potential_state_for_score_network() {
	this->network_inputs_potential_state_indexes.clear();

	this->halt_network->remove_potentials();
	this->no_halt_network->remove_potentials();
}

SolutionNode* SolutionNodeLoopEnd::activate(Problem& problem,
											double* state_vals,
											bool* states_on,
											vector<SolutionNode*>& loop_scopes,
											vector<int>& loop_scope_counts,
											int visited_count,
											SolutionNode* explore_node,
											int& explore_type,
											double* potential_state_vals,
											bool* potential_states_on,
											vector<NetworkHistory*>& network_historys) override {
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

	double halt_score;
	vector<NetworkHistory*> halt_history;
	double no_halt_score;
	vector<NetworkHistory*> no_halt_history;
	if (explore_type == EXPLORE_TYPE_STATE) {
		vector<int> potentials_on;
		vector<double> potential_vals;
		for (int p_index = 0; p_index < (int)this->network_inputs_potential_state_indexes.size(); p_index++) {
			if (potential_states_on[this->network_inputs_potential_state_indexes[p_index]]) {
				potentials_on.push_back(p_index);
				potential_vals.push_back(potential_state_vals[this->>network_inputs_potential_state_indexes[p_index]]);
			}
		}
		this->halt_network->mtx.lock();
		this->halt_network->activate(inputs,
									 potentials_on,
									 potential_vals,
									 halt_history);
		halt_score = this->halt_network->output->acti_vals[0];
		this->halt_network->mtx.unlock();
		this->no_halt_network->mtx.lock();
		this->no_halt_network->activate(inputs,
										potentials_on,
										potential_vals,
										no_halt_history);
		no_halt_score = this->no_halt_network->output->acti_vals[0];
		this->no_halt_network->mtx.unlock();
	} else {
		this->halt_network->mtx.lock();
		this->halt_network->activate(inputs, halt_history);
		halt_score = this->halt_network->output->acti_vals[0];
		this->halt_network->mtx.unlock();
		this->no_halt_network->mtx.lock();
		this->no_halt_network->activate(inputs, no_halt_history);
		no_halt_score = this->no_halt_network->output->acti_vals[0];
		this->no_halt_network->mtx.unlock();
	}

	if (visited_count == 0 && explore_node == NULL) {
		if (randuni() < 1.0/this->average_future_nodes) {
			explore_node = this;
			explore_type = EXPLORE_TYPE_PATH;
		}
	}

	if (explore_node == this) {
		if (halt_score < this->average && no_halt_score < this->average) {
			// explore path
		}
	}

	SolutionNode* next;
	if (loop_scopes.back() == this) {
		if (loop_scope_counts.back() < 20 && (no_halt > halt || rand()%20 == 0)) {
			next = no_halt;

			network_historys.push_back(no_halt_history[0]);
			delete halt_history[0];

			loop_scope_counts.back()++;
		} else {
			next = halt;

			network_historys.push_back(halt_history[0]);
			delete no_halt_history[0];

			loop_scopes.pop_back();
			loop_scope_counts.pop_back();

			for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
				states_on[this->start->scope_states_on[o_index]] = false;
			}
		}
	} else {
		if ((no_halt > halt || rand()%20 == 0)) {
			next = no_halt;

			network_historys.push_back(no_halt_history[0]);
			delete halt_history[0];

			loop_scopes.push_back(this);
			loop_scope_counts.push_back(1);
		} else {
			next = halt;

			network_historys.push_back(halt_history[0]);
			delete no_halt_history[0];

			for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
				states_on[this->start->scope_states_on[o_index]] = false;
			}
		}
	}

	return next;
}

void SolutionNodeLoopEnd::backprop(double score,
								   SolutionNode* explore_node,
								   int& explore_type,
								   double* potential_state_errors,
								   bool* potential_states_on,
								   vector<NetworkHistory*>& network_historys) override {
	NetworkHistory* network_history = network_historys.back();

	if (network_history->network == this->halt_network) {
		this->halt_network->mtx.lock();

		network_history->reset_weights();

		vector<int> potentials_on = network_history->potentials_on;

		vector<double> errors;
		if (score == 1.0) {
			if (this->halt_network->output->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->halt_network->output->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}

		this->halt_network->backprop(errors, potentials_on);

		for (int o_index = 0; o_index < (int)potentials_on.size(); o_index++) {
			potential_state_errors[this->network_inputs_potential_state_indexes[potentials_on[o_index]]] += \
				this->halt_network->potential_inputs[potentials_on[o_index]]->errors[0];
			this->halt_network->potential_inputs[potentials_on[o_index]]->errors[0] = 0.0;
		}

		this->halt_network->mtx.unlock();
	} else {
		this->no_halt_network->mtx.lock();

		network_history->reset_weights();

		vector<int> potentials_on = network_history->potentials_on;

		vector<double> errors;
		if (score == 1.0) {
			if (this->no_halt_network->output->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->no_halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->no_halt_network->output->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->no_halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}

		this->no_halt_network->backprop(errors, potentials_on);

		for (int p_index = 0; p_index < (int)potentials_on.size(); p_index++) {
			potential_state_errors[this->network_inputs_potential_state_indexes[potentials_on[p_index]]] += \
				this->no_halt_network->potential_inputs[potentials_on[o_index]]->errors[0];
			this->no_halt_network->potential_inputs[potentials_on[o_index]]->errors[0] = 0.0;
		}

		this->no_halt_network->mtx.unlock();
	}

	delete network_history;
	network_historys.pop_back();

	// clear state errors
}
