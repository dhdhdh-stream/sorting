#include "world_model.h"

#include "constants.h"
#include "globals.h"
#include "world_state.h"

using namespace std;

const int STARTING_NUM_STATES = 2;

WorldModel::WorldModel() {
	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (int s_index = 0; s_index < STARTING_NUM_STATES; s_index++) {
		WorldState* world_state = new WorldState();

		world_state->average_val = distribution(generator);

		world_state->transitions = vector<vector<double>>(NUM_ACTIONS);
		for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
			vector<double> likelihoods(STARTING_NUM_STATES);

			double sum_likelihood = 0.0;
			for (int s_index = 0; s_index < STARTING_NUM_STATES; s_index++) {
				likelihoods[s_index] = distribution(generator);

				sum_likelihood += likelihoods[s_index];
			}

			for (int s_index = 0; s_index < STARTING_NUM_STATES; s_index++) {
				likelihoods[s_index] /= sum_likelihood;
			}

			world_state->transitions[a_index] = likelihoods;
		}

		this->states.push_back(world_state);
	}

	{
		this->starting_likelihood = vector<double>(STARTING_NUM_STATES);

		double sum_likelihood = 0.0;
		for (int s_index = 0; s_index < STARTING_NUM_STATES; s_index++) {
			this->starting_likelihood[s_index] = distribution(generator);

			sum_likelihood += this->starting_likelihood[s_index];
		}

		for (int s_index = 0; s_index < STARTING_NUM_STATES; s_index++) {
			this->starting_likelihood[s_index] /= sum_likelihood;
		}
	}
}

WorldModel::WorldModel(WorldModel* original) {
	for (int s_index = 0; s_index < (int)original->states.size(); s_index++) {
		this->states.push_back(new WorldState(original->states[s_index]));
	}

	this->starting_likelihood = original->starting_likelihood;
}

WorldModel::~WorldModel() {
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		delete this->states[s_index];
	}
}

void WorldModel::split_state(int state_index) {
	this->states.push_back(new WorldState(this->states[state_index]));
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		this->states[s_index]->split_state(state_index);
	}

	{
		uniform_real_distribution<double> distribution(0.0, 1.0);
		double original_likelihood = distribution(generator);
		double new_likelihood = distribution(generator);
		double sum_likelihood = original_likelihood + new_likelihood;

		double original_starting = this->starting_likelihood[state_index];

		this->starting_likelihood[state_index] = original_starting * original_likelihood / sum_likelihood;
		this->starting_likelihood.push_back(original_starting * new_likelihood / sum_likelihood);
	}
}
