#include "instance_scores.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int TARGET_LEFT = 0;
const int TARGET_RIGHT = 1;

const int ACTION_LEFT = 0;
const int ACTION_RIGHT = 1;
const int ACTION_CLICK = 2;

InstanceScores::InstanceScores() {
	this->epoch = 0;

	this->curr_index = 3;
	uniform_int_distribution<int> target_distribution(0, 1);
	if (target_distribution(generator) == 0) {
		this->curr_target = TARGET_LEFT;
	} else {
		this->curr_target = TARGET_RIGHT;
	}
	this->last_success = 0;

	this->final_score = 0.0;
}

vector<double> InstanceScores::get_observations() {
	vector<double> obs;

	obs.push_back(this->curr_index);

	switch (this->curr_target) {
	case TARGET_LEFT:
		obs.push_back(-1.0);
		break;
	case TARGET_RIGHT:
		obs.push_back(1.0);
		break;
	}

	obs.push_back(this->last_success);

	obs.push_back(this->epoch);

	return obs;
}

void InstanceScores::perform_action(int action) {
	switch (action) {
	case ACTION_LEFT:
		this->curr_index--;
		if (this->curr_index < 0) {
			this->curr_index = 0;
		}
		break;
	case ACTION_RIGHT:
		this->curr_index++;
		if (this->curr_index > 6) {
			this->curr_index = 6;
		}
		break;
	case ACTION_CLICK:
		switch (this->curr_target) {
		case TARGET_LEFT:
			if (this->curr_index == 1) {
				this->final_score += 1.0;

				this->last_success = 1.0;
			} else {
				this->final_score -= 1.0;

				this->last_success = -1.0;
			}
			break;
		case TARGET_RIGHT:
			if (this->curr_index == 5) {
				this->final_score += 1.0;

				this->last_success = 1.0;
			} else {
				this->final_score -= 1.0;

				this->last_success = -1.0;
			}
			break;
		}

		this->epoch++;

		uniform_int_distribution<int> target_distribution(0, 1);
		if (target_distribution(generator) == 0) {
			this->curr_target = TARGET_LEFT;
		} else {
			this->curr_target = TARGET_RIGHT;
		}

		break;
	}
}

double InstanceScores::score_result() {
	return this->final_score;
}

void InstanceScores::print() {
	cout << "this->epoch: " << this->epoch << endl;
	cout << "this->final_score: " << this->final_score << endl;
}

Problem* TypeInstanceScores::get_problem() {
	return new InstanceScores();
}

int TypeInstanceScores::num_obs() {
	return 4;
}

int TypeInstanceScores::num_possible_actions() {
	return 3;
}
