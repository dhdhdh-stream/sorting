#include "world_state.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "utilities.h"

using namespace std;

WorldState::WorldState() {
	// do nothing
}

void WorldState::forward(double obs,
						 vector<double>& curr_likelihoods,
						 int action,
						 vector<double>& next_likelihoods) {
	double z_score = -abs((obs - this->average_val) / this->average_standard_deviation);
	double obs_likelihood = 2.0 * cumulative_normal(z_score);
	double state_likelihood = curr_likelihoods[this->id] * obs_likelihood;
	for (int s_index = 0; s_index < (int)next_likelihoods.size(); s_index++) {
		double transition_likelihood = this->transitions[action][s_index];
		next_likelihoods[s_index] += state_likelihood * transition_likelihood;
	}
}

void WorldState::backward(vector<double>& curr_likelihoods,
						  int action,
						  double obs,
						  vector<double>& next_likelihoods) {
	double z_score = -abs((obs - this->average_val) / this->average_standard_deviation);
	double obs_likelihood = 2.0 * cumulative_normal(z_score);
	for (int s_index = 0; s_index < (int)next_likelihoods.size(); s_index++) {
		double transition_likelihood = this->transitions[action][s_index];
		next_likelihoods[this->id] += curr_likelihoods[s_index]
			* transition_likelihood
			* obs_likelihood;
	}
}
