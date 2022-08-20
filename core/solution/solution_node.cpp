#include "solution_node.h"

#include <cmath>
#include <iostream>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

// SolutionNode::SolutionNode(Solution* solution,
// 						   int node_index) {
// 	this->solver = solver;
// 	this->node_index = node_index;

// 	this->average_score = 0.5;
// }

// SolutionNode::SolutionNode(Solver* solver,
// 						   int node_index,
// 						   ifstream& save_file) {
// 	this->solver = solver;
// 	this->node_index = node_index;

// 	string path_length_line;
// 	getline(save_file, path_length_line);
// 	this->path_length = stoi(path_length_line);

// 	string average_score_line;
// 	getline(save_file, average_score_line);
// 	this->average_score = stof(average_score_line);
// }

SolutionNode::~SolutionNode() {
	
}

void SolutionNode::increment_unique_future_nodes(int num_future_nodes) {
	this->average_unique_future_nodes = 0.9999*this->average_unique_future_nodes + 0.0001*num_future_nodes;
}

// void SolutionNode::save(ofstream& save_file) {
// 	save_file << this->average_score << endl;
// }

// void SolutionNode::save_for_display(ofstream& save_file) {
// 	save_file << this->average_score << endl;
// }

void SolutionNode::add_potential_state_for_score_network(vector<int> potential_state_indexes) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		this->network_inputs_potential_state_indexes.push_back(
			potential_state_indexes[ps_index]);
		this->score_network->add_potential();
	}
}

void SolutionNode::extend_state_for_score_network(vector<int> potential_state_indexes,
												  vector<int> new_state_indexes) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->network_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->network_inputs_state_indexes.push_back(new_state_indexes[ps_index]);
				this->score_network->extend_with_potential(pi_index);
				break;
			}
		}
	}
}

void SolutionNode::reset_potential_state_for_score_network(vector<int> potential_state_indexes) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->network_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->score_network->reset_potential(pi_index);
			}
		}
	}
}

void SolutionNode::clear_potential_state_for_score_network() {
	this->network_inputs_potential_state_indexes.clear();
	this->score_network->remove_potentials();
}

double SolutionNode::activate_score_network(Problem& problem,
											double* state_vals,
											bool* states_on,
											bool backprop,
											vector<NetworkHistory*>& network_historys) {
	vector<double> score_network_inputs;
	double curr_observations = problem.get_observation();
	score_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	double score;
	if (backprop) {
		this->score_network->mtx.lock();
		this->score_network->activate(score_network_inputs, network_historys);
		score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();
	} else {
		this->score_network->mtx.lock();
		this->score_network->activate(score_network_inputs);
		score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_score_network(double score,
										  double* state_errors,
										  bool* states_on,
										  vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->score_network->mtx.lock();

	network_history->reset_weights();

	vector<double> score_network_errors;
	if (score == 1.0) {
		if (this->score_network->output->acti_vals[0] < 1.0) {
			score_network_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	} else {
		if (this->score_network->output->acti_vals[0] > 0.0) {
			score_network_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	}
	this->score_network->backprop(score_network_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->score_network->input->errors[1+i_index];
		}
		this->score_network->input->errors[1+i_index] = 0.0;
	}

	this->score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNode::backprop_score_network_errors_with_no_weight_change(
		double score,
		double* state_errors,
		bool* states_on,
		vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->score_network->mtx.lock();

	network_history->reset_weights();

	vector<double> score_network_errors;
	if (score == 1.0) {
		if (this->score_network->output->acti_vals[0] < 1.0) {
			score_network_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	} else {
		if (this->score_network->output->acti_vals[0] > 0.0) {
			score_network_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	}
	this->score_network->backprop_errors_with_no_weight_change(score_network_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->score_network->input->errors[1+i_index];
		}
		this->score_network->input->errors[1+i_index] = 0.0;
	}

	this->score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNode::activate_score_network_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		bool* potential_states_on,
		bool backprop,
		vector<NetworkHistory*>& network_historys) {
	vector<double> score_network_inputs;
	double curr_observations = problem.get_observation();
	score_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	vector<int> potentials_on;
	vector<double> potential_vals;
	for (int p_index = 0; p_index < (int)this->network_inputs_potential_state_indexes.size(); p_index++) {
		if (potential_states_on[this->network_inputs_potential_state_indexes[p_index]]) {
			potentials_on.push_back(p_index);
			potential_vals.push_back(potential_state_vals[this->network_inputs_potential_state_indexes[p_index]]);
		}
	}

	double score;
	if (potentials_on.size() > 0) {
		if (backprop) {
			this->score_network->mtx.lock();
			this->score_network->activate(score_network_inputs,
										  potentials_on,
										  potential_vals,
										  network_historys);
			score = this->score_network->output->acti_vals[0];
			this->score_network->mtx.unlock();
		} else {
			this->score_network->mtx.lock();
			this->score_network->activate(score_network_inputs,
										  potentials_on,
										  potential_vals);
			score = this->score_network->output->acti_vals[0];
			this->score_network->mtx.unlock();
		}
	} else {
		this->score_network->mtx.lock();
		this->score_network->activate(score_network_inputs);
		score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_score_network_with_potential(
		double score,
		double* potential_state_errors,
		vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	if (network_history->network == this->score_network) {
		this->score_network->mtx.lock();

		network_history->reset_weights();

		vector<int> potentials_on = network_history->potentials_on;

		vector<double> score_network_errors;
		if (score == 1.0) {
			if (this->score_network->output->acti_vals[0] < 1.0) {
				score_network_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
			} else {
				score_network_errors.push_back(0.0);
			}
		} else {
			if (this->score_network->output->acti_vals[0] > 0.0) {
				score_network_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
			} else {
				score_network_errors.push_back(0.0);
			}
		}
		this->score_network->backprop(score_network_errors,
									  potentials_on);

		for (int o_index = 0; o_index < (int)potentials_on.size(); o_index++) {
			potential_state_errors[this->network_inputs_potential_state_indexes[potentials_on[o_index]]] += \
				this->score_network->potential_inputs[potentials_on[o_index]]->errors[0];
			this->score_network->potential_inputs[potentials_on[o_index]]->errors[0] = 0.0;
		}

		this->score_network->mtx.unlock();

		delete network_history;
		network_historys.pop_back();
	}
}

double SolutionNode::activate_explore_if_network(Problem& problem,
												 double* state_vals,
												 bool* states_on,
												 bool backprop,
												 vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	double score;
	if (backprop) {
		this->explore_if_network->mtx.lock();
		this->explore_if_network->activate(explore_network_inputs, network_historys);
		score = this->explore_if_network->output->acti_vals[0];
		this->explore_if_network->mtx.unlock();
	} else {
		this->explore_if_network->mtx.lock();
		this->explore_if_network->activate(explore_network_inputs);
		score = this->explore_if_network->output->acti_vals[0];
		this->explore_if_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_explore_if_network(double score,
											   double* state_errors,
											   bool* states_on,
											   vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->explore_if_network->mtx.lock();

	network_history->reset_weights();

	vector<double> explore_network_errors;
	if (score == 1.0) {
		if (this->explore_if_network->output->acti_vals[0] < 1.0) {
			explore_network_errors.push_back(1.0 - this->explore_if_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	} else {
		if (this->explore_if_network->output->acti_vals[0] > 0.0) {
			explore_network_errors.push_back(0.0 - this->explore_if_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	}
	this->explore_if_network->backprop(explore_network_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->explore_if_network->input->errors[1+i_index];
		}
		this->explore_if_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_if_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNode::activate_explore_halt_network(Problem& problem,
												   double* state_vals,
												   bool* states_on,
												   bool backprop,
												   vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	if (backprop) {
		this->explore_halt_network->mtx.lock();
		this->explore_halt_network->activate(explore_network_inputs, network_historys);
		score = this->explore_halt_network->output->acti_vals[0];
		this->explore_halt_network->mtx.unlock();
	} else {
		this->explore_halt_network->mtx.lock();
		this->explore_halt_network->activate(explore_network_inputs);
		score = this->explore_halt_network->output->acti_vals[0];
		this->explore_halt_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_explore_halt_network(double score,
												 double* state_errors,
												 bool* states_on,
												 vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->explore_halt_network->mtx.lock();

	network_history->reset_weights();

	vector<double> explore_network_errors;
	if (score == 1.0) {
		if (this->explore_halt_network->output->acti_vals[0] < 1.0) {
			explore_network_errors.push_back(1.0 - this->explore_halt_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	} else {
		if (this->explore_halt_network->output->acti_vals[0] > 0.0) {
			explore_network_errors.push_back(0.0 - this->explore_halt_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	}
	this->explore_halt_network->backprop(explore_network_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->explore_halt_network->input->errors[1+i_index];
		}
		this->explore_halt_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_halt_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNode::activate_explore_no_halt_network(Problem& problem,
													  double* state_vals,
													  bool* states_on,
													  bool backprop,
													  vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}
	
	double score;
	if (backprop) {
		this->explore_no_halt_network->mtx.lock();
		this->explore_no_halt_network->activate(explore_network_inputs, network_historys);
		score = this->explore_no_halt_network->output->acti_vals[0];
		this->explore_no_halt_network->mtx.unlock();
	} else {
		this->explore_no_halt_network->mtx.lock();
		this->explore_no_halt_network->activate(explore_network_inputs);
		score = this->explore_no_halt_network->output->acti_vals[0];
		this->explore_no_halt_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_explore_no_halt_network(double score,
													double* states_errors,
													bool* states_on,
													std::vector<NetworkHistory*>& network_historys) {
	this->explore_no_halt_network->mtx.lock();

	network_history->reset_weights();

	vector<double> explore_network_errors;
	if (score == 1.0) {
		if (this->explore_no_halt_network->output->acti_vals[0] < 1.0) {
			explore_network_errors.push_back(1.0 - this->explore_no_halt_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	} else {
		if (this->explore_no_halt_network->output->acti_vals[0] > 0.0) {
			explore_network_errors.push_back(0.0 - this->explore_no_halt_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	}
	this->explore_no_halt_network->backprop(explore_network_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->explore_no_halt_network->input->errors[1+i_index];
		}
		this->explore_no_halt_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_no_halt_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}
