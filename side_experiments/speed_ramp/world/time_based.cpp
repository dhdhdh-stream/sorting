#include "time_based.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int MAX_EPOCH = 20;
const int MAX_ITERS = 10;

const int TARGET_LEFT = 0;
const int TARGET_RIGHT = 1;

const int ACTION_LEFT = 0;
const int ACTION_RIGHT = 1;
const int ACTION_CLICK = 2;

TimeBased::TimeBased() {
	this->epoch = 0;
	this->iter = 0;

	this->curr_index = 3;
	uniform_int_distribution<int> target_distribution(0, 1);
	if (target_distribution(generator) == 0) {
		this->curr_target = TARGET_LEFT;
	} else {
		this->curr_target = TARGET_RIGHT;
	}
	this->curr_clicked = false;
	this->curr_success = false;

	this->final_score = 0.0;
}

vector<double> TimeBased::get_observations() {
	vector<double> obs;

	if (this->epoch >= MAX_EPOCH) {
		obs.push_back(this->final_score);
	} else {
		if (this->iter >= MAX_ITERS) {
			if (this->iter % 2 == 0) {
				obs.push_back(0.0);
			} else {
				obs.push_back(5 * this->curr_success);
			}
		} else {
			switch (this->curr_target) {
			case TARGET_LEFT:
				obs.push_back(-1.0);
				break;
			case TARGET_RIGHT:
				obs.push_back(1.0);
				break;
			}
		}
	}

	return obs;
}

void TimeBased::perform_action(int action) {
	if (this->epoch < MAX_EPOCH) {
		if (this->iter < MAX_ITERS) {
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
				if (this->curr_clicked) {
					this->curr_success = false;
				} else {
					switch (this->curr_target) {
					case TARGET_LEFT:
						if (this->curr_index == 1) {
							this->curr_success = 1;
							this->final_score += 1.0;
						} else {
							this->curr_success = -1;
							this->final_score -= 1.0;
						}
						break;
					case TARGET_RIGHT:
						if (this->curr_index == 5) {
							this->curr_success = 1;
							this->final_score += 1.0;
						} else {
							this->curr_success = -1;
							this->final_score -= 1.0;
						}
						break;
					}

					this->curr_clicked = true;
				}
				break;
			}
		}

		this->iter++;
		if (this->iter >= MAX_ITERS) {
			this->iter = 0;

			this->curr_index = 3;
			uniform_int_distribution<int> target_distribution(0, 1);
			if (target_distribution(generator) == 0) {
				this->curr_target = TARGET_LEFT;
			} else {
				this->curr_target = TARGET_RIGHT;
			}
			this->curr_clicked = false;
			this->curr_success = false;

			this->epoch++;
		}
	} else {
		this->final_score -= 0.01;
	}
}

double TimeBased::score_result() {
	return this->final_score;
}

void TimeBased::print() {
	cout << "this->epoch: " << this->epoch << endl;
	cout << "this->iter: " << this->iter << endl;
	cout << "this->final_score: " << this->final_score << endl;
}

Problem* TypeTimeBased::get_problem() {
	return new TimeBased();
}

int TypeTimeBased::num_obs() {
	return 1;
}

int TypeTimeBased::num_possible_actions() {
	return 3;
}
