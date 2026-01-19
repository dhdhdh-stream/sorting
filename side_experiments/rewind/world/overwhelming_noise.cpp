#include "overwhelming_noise.h"

#include <iostream>

#include "globals.h"
#include "utilities.h"

using namespace std;

const int NUM_OBS = 20;

OverwhelmingNoise::OverwhelmingNoise() {
	this->starting_seed = rand();
	this->current_seed = this->starting_seed;

	this->curr_obs = vector<double>(NUM_OBS);
	for (int o_index = 0; o_index < NUM_OBS; o_index++) {
		this->current_seed = xorshift(this->current_seed);
		this->curr_obs[o_index] = this->current_seed % 2;
	}

	this->score = 0.0;

	uniform_int_distribution<int> noise_distribution(-1000, 1000);
	this->noise = noise_distribution(generator);
}

vector<double> OverwhelmingNoise::get_observations() {
	return this->curr_obs;
}

void OverwhelmingNoise::perform_action(int action) {
	if (action == this->curr_obs[0]) {
		this->score += 1.0;
	} else {
		this->score -= 1.0;
	}

	this->curr_obs = vector<double>(NUM_OBS);
	for (int o_index = 0; o_index < NUM_OBS; o_index++) {
		this->current_seed = xorshift(this->current_seed);
		this->curr_obs[o_index] = this->current_seed % 2;
	}
}

double OverwhelmingNoise::score_result() {
	return this->score + this->noise;
}

Problem* OverwhelmingNoise::copy_and_reset() {
	OverwhelmingNoise* new_problem = new OverwhelmingNoise();

	new_problem->starting_seed = this->starting_seed;
	new_problem->noise = this->noise;

	new_problem->current_seed = new_problem->starting_seed;
	for (int o_index = 0; o_index < NUM_OBS; o_index++) {
		new_problem->current_seed = xorshift(new_problem->current_seed);
		new_problem->curr_obs[o_index] = new_problem->current_seed % 2;
	}

	return new_problem;
}

Problem* OverwhelmingNoise::copy_snapshot() {
	OverwhelmingNoise* new_problem = new OverwhelmingNoise();

	new_problem->starting_seed = this->starting_seed;
	new_problem->current_seed = this->current_seed;
	new_problem->curr_obs = this->curr_obs;
	new_problem->score = this->score;
	new_problem->noise = this->noise;

	return new_problem;
}

void OverwhelmingNoise::print() {
	cout << "this->score: " << this->score << endl;
	cout << "this->noise: " << this->noise << endl;
}

Problem* TypeOverwhelmingNoise::get_problem() {
	return new OverwhelmingNoise();
}

int TypeOverwhelmingNoise::num_obs() {
	return NUM_OBS;
}

int TypeOverwhelmingNoise::num_possible_actions() {
	return 2;
}
