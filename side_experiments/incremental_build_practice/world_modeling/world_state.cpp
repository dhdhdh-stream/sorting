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
}

void WorldState::split_state(int state_index) {
	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		double original_likelihood = distribution(generator);
		double new_likelihood = distribution(generator);
		double sum_likelihood = original_likelihood + new_likelihood;

		double original_transition = this->transitions[a_index][state_index];

		this->transitions[a_index][state_index] = original_transition * original_likelihood / sum_likelihood;
		this->transitions[a_index].push_back(original_transition * new_likelihood / sum_likelihood);
	}
}

void WorldState::save_for_display(ofstream& output_file) {
	output_file << this->average_val << endl;

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		for (int s_index = 0; s_index < (int)this->transitions[a_index].size(); s_index++) {
			output_file << this->transitions[a_index][s_index] << endl;
		}
	}
}
