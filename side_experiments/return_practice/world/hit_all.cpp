#include "hit_all.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int WORLD_SIZE = 7;
const int START_INDEX = 3;

HitAll::HitAll() {
	this->world = vector<int>(WORLD_SIZE);
	uniform_int_distribution<int> type_distribution(0, 1);
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		this->world[i_index] = type_distribution(generator);
	}

	this->curr_index = START_INDEX;
}

vector<double> HitAll::get_observations() {
	vector<double> obs;

	if (this->curr_index < 0
			|| this->curr_index >= WORLD_SIZE) {
		obs.push_back(-1.0);
	} else {
		obs.push_back(this->world[this->curr_index]);
	}

	return obs;
}

void HitAll::perform_action(int action) {
	switch (action) {
	case 0:
		this->curr_index--;
		if (this->curr_index < -1) {
			this->curr_index = -1;
		}
		break;
	case 1:
		this->curr_index++;
		if (this->curr_index > WORLD_SIZE) {
			this->curr_index = WORLD_SIZE;
		}
		break;
	case 2:
		if (this->curr_index >= 0 && this->curr_index < WORLD_SIZE) {
			this->world[this->curr_index] = (this->world[this->curr_index] + 1) % 2;
		}
		break;
	}
}

double HitAll::score_result() {
	double sum_score = 0.0;
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		sum_score += this->world[i_index];
	}
	return sum_score;
}

Problem* TypeHitAll::get_problem() {
	return new HitAll();
}

int TypeHitAll::num_obs() {
	return 1;
}

int TypeHitAll::num_possible_actions() {
	return 3;
}
