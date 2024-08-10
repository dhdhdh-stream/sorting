#include "world_state.h"

#include "constants.h"
#include "globals.h"

using namespace std;

WorldState::WorldState() {
	// do nothing
}

WorldState::WorldState(WorldState* original) {
	this->average_val = original->average_val;

	this->transitions = original->transitions;
	this->unknown_transitions = original->unknown_transitions;
}

WorldState::WorldState(ifstream& input_file) {
	string average_val_line;
	getline(input_file, average_val_line);
	this->average_val = stod(average_val_line);

	string num_states_line;
	getline(input_file, num_states_line);
	int num_states = stoi(num_states_line);

	this->transitions = vector<vector<double>>(NUM_ACTIONS);
	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		this->transitions[a_index] = vector<double>(num_states);
		for (int s_index = 0; s_index < num_states; s_index++) {
			string likelihood_line;
			getline(input_file, likelihood_line);
			this->transitions[a_index][s_index] = stod(likelihood_line);
		}
	}

	this->unknown_transitions = vector<double>(NUM_ACTIONS);
	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		string likelihood_line;
		getline(input_file, likelihood_line);
		this->unknown_transitions[a_index] = stod(likelihood_line);
	}
}

void WorldState::add_state() {
	int num_states = (int)this->transitions[0].size();

	double scale = (double)num_states / (double)(num_states + 1);

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		for (int s_index = 0; s_index < num_states; s_index++) {
			this->transitions[a_index][s_index] *= scale;
		}

		this->unknown_transitions[a_index] *= scale;

		this->transitions[a_index].push_back(1.0 / (double)(num_states+1));
	}
}

void WorldState::save(ofstream& output_file) {
	output_file << this->average_val << endl;

	output_file << this->transitions[0].size() << endl;

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		for (int s_index = 0; s_index < (int)this->transitions[a_index].size(); s_index++) {
			output_file << this->transitions[a_index][s_index] << endl;
		}
	}

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		output_file << this->unknown_transitions[a_index] << endl;
	}
}

void WorldState::save_for_display(ofstream& output_file) {
	output_file << this->average_val << endl;

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		for (int s_index = 0; s_index < (int)this->transitions[a_index].size(); s_index++) {
			output_file << this->transitions[a_index][s_index] << endl;
		}
	}

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		output_file << this->unknown_transitions[a_index] << endl;
	}
}
