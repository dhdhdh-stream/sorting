#include "tunnel_practice.h"

#include <algorithm>
#include <iostream>

#include "globals.h"

using namespace std;

const int STARTING_INDEX = 2;
const int WORLD_SIZE = 5;

const int NUM_TARGETS = 20;

const int NUM_NOISE = 10;

TunnelPractice::TunnelPractice() {
	this->curr_index = STARTING_INDEX;

	uniform_int_distribution<int> target_distribution(0, 1);
	for (int i_index = 0; i_index < NUM_TARGETS; i_index++) {
		this->targets.push_back(target_distribution(generator));
	}
	this->curr_target_index = 0;

	this->score = 0.0;

	uniform_int_distribution<int> noise_distribution(-10, 10);
	for (int i_index = 0; i_index < NUM_NOISE; i_index++) {
		this->noise.push_back(noise_distribution(generator));
	}
}

vector<double> TunnelPractice::get_observations() {
	vector<double> obs;

	obs.push_back(this->curr_index);
	if (this->curr_target_index >= NUM_TARGETS) {
		obs.push_back(-1);
	} else {
		obs.push_back(this->targets[this->curr_target_index]);
	}

	obs.push_back(this->score);
	for (int i_index = 0; i_index < NUM_NOISE; i_index++) {
		obs.push_back(this->noise[i_index]);
	}

	return obs;
}

void TunnelPractice::perform_action(int action) {
	switch (action) {
	case TUNNEL_PRACTICE_ACTION_LEFT:
		this->curr_index--;
		if (this->curr_index < -1) {
			this->curr_index = -1;
		}
		break;
	case TUNNEL_PRACTICE_ACTION_RIGHT:
		this->curr_index++;
		if (this->curr_index > WORLD_SIZE) {
			this->curr_index = WORLD_SIZE;
		}
		break;
	case TUNNEL_PRACTICE_ACTION_CLICK:
		if (this->curr_target_index >= NUM_TARGETS) {
			this->score -= 1.0;
		} else {
			if (this->targets[this->curr_target_index] == 0) {
				if (this->curr_index == 0) {
					this->score += 1.0;

					this->curr_target_index++;
				} else {
					this->score -= 1.0;
				}
			} else {
				if (this->curr_index == WORLD_SIZE-1) {
					this->score += 1.0;

					this->curr_target_index++;
				} else {
					this->score -= 1.0;
				}
			}
		}
		break;
	}
}

double TunnelPractice::score_result() {
	double sum_scores = 0.0;

	sum_scores += this->score;
	for (int i_index = 0; i_index < NUM_NOISE; i_index++) {
		sum_scores += this->noise[i_index];
	}

	return sum_scores;
}

Problem* TunnelPractice::copy_and_reset() {
	TunnelPractice* new_problem = new TunnelPractice();

	new_problem->targets = this->targets;

	new_problem->noise = this->noise;

	return new_problem;
}

Problem* TunnelPractice::copy_snapshot() {
	TunnelPractice* new_problem = new TunnelPractice();

	new_problem->curr_index = this->curr_index;
	new_problem->targets = this->targets;
	new_problem->curr_target_index = this->curr_target_index;
	new_problem->score = this->score;
	new_problem->noise = this->noise;

	return new_problem;
}

void TunnelPractice::print() {
	cout << "this->curr_target_index: " << this->curr_target_index << endl;
	cout << "this->score: " << this->score << endl;
}

Problem* TypeTunnelPractice::get_problem() {
	return new TunnelPractice();
}

int TypeTunnelPractice::num_obs() {
	return 13;
}

int TypeTunnelPractice::num_possible_actions() {
	return 3;
}
