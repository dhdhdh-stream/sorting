#include "world_state.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "world_model.h"

using namespace std;

WorldState::WorldState() {
	// do nothing
}

void WorldState::forward(double obs,
						 vector<double>& curr_likelihoods,
						 int action,
						 vector<double>& next_likelihoods,
						 double& next_unknown_likelihood) {
	double state_likelihood = curr_likelihoods[this->id]
		* (1.0 - abs(obs - this->average_val));
	if (this->transitions[action].size() > 0) {
		for (int t_index = 0; t_index < (int)this->transitions[action].size(); t_index++) {
			int state_index = this->transitions[action][t_index].first->id;
			double transition_likelihood = this->transitions[action][t_index].second;
			next_likelihoods[state_index] += state_likelihood * transition_likelihood;
		}
	} else {
		next_unknown_likelihood += state_likelihood;
	}
}

void WorldState::backward(vector<double>& curr_likelihoods,
						  double curr_unknown_likelihood,
						  int action,
						  double obs,
						  vector<double>& next_likelihoods) {
	if (this->transitions[action].size() > 0) {
		for (int t_index = 0; t_index < (int)this->transitions[action].size(); t_index++) {
			int state_index = this->transitions[action][t_index].first->id;
			double transition_likelihood = this->transitions[action][t_index].second;
			next_likelihoods[this->id] += curr_likelihoods[state_index]
				* transition_likelihood
				* (1.0 - abs(obs - this->average_val));
		}
	} else {
		next_likelihoods[this->id] += curr_unknown_likelihood
			* (1.0 - abs(obs - this->average_val));
	}
}

void WorldState::save(ofstream& output_file) {
	output_file << this->average_val << endl;

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		output_file << this->transitions[a_index].size() << endl;
		for (int t_index = 0; t_index < (int)this->transitions[a_index].size(); t_index++) {
			output_file << this->transitions[a_index][t_index].first->id << endl;
			output_file << this->transitions[a_index][t_index].second << endl;
		}
	}
}

void WorldState::load(ifstream& input_file) {
	string average_val_line;
	getline(input_file, average_val_line);
	this->average_val = stod(average_val_line);

	this->transitions = vector<vector<pair<WorldState*,double>>>(NUM_ACTIONS);
	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		string num_transitions_line;
		getline(input_file, num_transitions_line);
		int num_transitions = stoi(num_transitions_line);
		for (int t_index = 0; t_index < num_transitions; t_index++) {
			string state_index_line;
			getline(input_file, state_index_line);
			int state_index = stoi(state_index_line);

			string likelihood_line;
			getline(input_file, likelihood_line);
			double likelihood = stod(likelihood_line);

			this->transitions[a_index].push_back({
				this->parent->states[state_index],
				likelihood});
		}
	}
}

void WorldState::copy_from(WorldState* original) {
	this->average_val = original->average_val;

	this->transitions = original->transitions;
	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		for (int t_index = 0; t_index < (int)this->transitions[a_index].size(); t_index++) {
			this->transitions[a_index][t_index].first =
				this->parent->states[this->transitions[a_index][t_index].first->id];
		}
	}
}

void WorldState::save_for_display(ofstream& output_file) {
	output_file << this->average_val << endl;

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		output_file << this->transitions[a_index].size() << endl;
		for (int t_index = 0; t_index < (int)this->transitions[a_index].size(); t_index++) {
			output_file << this->transitions[a_index][t_index].first->id << endl;
			output_file << this->transitions[a_index][t_index].second << endl;
		}
	}
}
