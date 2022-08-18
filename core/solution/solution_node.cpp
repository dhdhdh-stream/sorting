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

// void SolutionNode::save(ofstream& save_file) {
// 	save_file << this->average_score << endl;
// }

// void SolutionNode::save_for_display(ofstream& save_file) {
// 	save_file << this->average_score << endl;
// }

double SolutionNode::activate_score_network(Problem& problem,
											double* state_vals,
											bool* states_on,
											int& explore_type,
											double* potential_state_vals,
											bool* potential_states_on,
											std::vector<NetworkHistory*>& network_historys) {
	vector<double> score_network_inputs;
	double curr_observations = problem.get_observation();
	score_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->score_network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	double score;

	vector<int> potentials_on;
	vector<double> potential_vals;
	if (explore_type == EXPLORE_TYPE_STATE) {
		for (int p_index = 0; p_index < (int)this->score_network_inputs_potential_state_indexes.size(); p_index++) {
			if (potential_states_on[this->score_network_inputs_potential_state_indexes[p_index]]) {
				potentials_on.push_back(p_index);
				potential_vals.push_back(potential_state_vals[this->score_network_inputs_potential_state_indexes[p_index]]);
			}
		}
	}

	if (this->explore_type == EXPLORE_TYPE_RE_EVAL
			|| this->explore_node_state == EXPLORE_NODE_STATE_LEARN
			|| potentials_on.size() > 0) {
		this->score_network->mtx.lock();
		this->score_network->activate(score_network_inputs,
									  potentials_on,
									  potential_vals,
									  network_historys);
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
										  double* potential_state_errors,
										  std::vector<NetworkHistory*>& network_historys) {
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
			potential_state_errors[this->score_network_inputs_potential_state_indexes[potentials_on[o_index]]] += \
				this->score_network->potential_inputs[potentials_on[o_index]]->errors[0];
			this->score_network->potential_inputs[potentials_on[o_index]]->errors[0] = 0.0;
		}

		this->score_network->mtx.unlock();

		delete network_history;
		network_historys.pop_back();
	}
}
