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
	double obs_likelihood;
	if (this->average_standard_deviation == 0.0) {
		if (obs == this->average_val) {
			obs_likelihood = 1.0;
		} else {
			obs_likelihood = 0.0;
		}
	} else {
		double z_score = -abs((obs - this->average_val) / this->average_standard_deviation);
		obs_likelihood = 2.0 * cumulative_normal(z_score);
	}
	double state_likelihood = curr_likelihoods[this->id] * obs_likelihood;
	for (int t_index = 0; t_index < (int)this->transitions[action].size(); t_index++) {
		int state_index = this->transitions[action][t_index].first;
		double transition_likelihood = this->transitions[action][t_index].second;
		next_likelihoods[state_index] += state_likelihood * transition_likelihood;
	}
}

void WorldState::backward(vector<double>& curr_likelihoods,
						  int action,
						  double obs,
						  vector<double>& next_likelihoods) {
	double obs_likelihood;
	if (this->average_standard_deviation == 0.0) {
		if (obs == this->average_val) {
			obs_likelihood = 1.0;
		} else {
			obs_likelihood = 0.0;
		}
	} else {
		double z_score = -abs((obs - this->average_val) / this->average_standard_deviation);
		obs_likelihood = 2.0 * cumulative_normal(z_score);
	}
	for (int t_index = 0; t_index < (int)this->transitions[action].size(); t_index++) {
		int state_index = this->transitions[action][t_index].first;
		double transition_likelihood = this->transitions[action][t_index].second;
		next_likelihoods[this->id] += curr_likelihoods[state_index]
			* transition_likelihood
			* obs_likelihood;
	}
}

void WorldState::transition_step(vector<double>& curr_likelihoods,
								 int action,
								 vector<double>& next_likelihoods) {
	for (int t_index = 0; t_index < (int)this->transitions[action].size(); t_index++) {
		int state_index = this->transitions[action][t_index].first;
		double transition_likelihood = this->transitions[action][t_index].second;
		next_likelihoods[state_index] += curr_likelihoods[this->id] * transition_likelihood;
	}
}

void WorldState::obs_step(vector<double>& curr_likelihoods,
						  double obs) {
	double obs_likelihood;
	if (this->average_standard_deviation == 0.0) {
		if (obs == this->average_val) {
			obs_likelihood = 1.0;
		} else {
			obs_likelihood = 0.0;
		}
	} else {
		double z_score = -abs((obs - this->average_val) / this->average_standard_deviation);
		obs_likelihood = 2.0 * cumulative_normal(z_score);
	}
	curr_likelihoods[this->id] *= obs_likelihood;
}

void WorldState::save_for_display(ofstream& output_file) {
	output_file << this->average_val << endl;

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		output_file << this->transitions[a_index].size() << endl;
		for (int t_index = 0; t_index < (int)this->transitions[a_index].size(); t_index++) {
			output_file << this->transitions[a_index][t_index].first << endl;
			output_file << this->transitions[a_index][t_index].second << endl;
		}
	}
}
