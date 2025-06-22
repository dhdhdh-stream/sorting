#include "simple.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int ACTION_UP = 0;
const int ACTION_RIGHT = 1;
const int ACTION_DOWN = 2;
const int ACTION_LEFT = 3;
const int ACTION_CLICK = 4;

const int NUM_TARGETS = 10;

Simple::Simple() {
	this->world = vector<vector<int>>(3, vector<int>(3));

	uniform_int_distribution<int> target_val_distribution(-10, 10);
	this->world[0][0] = target_val_distribution(generator);
	this->world[1][0] = 0.0;
	this->world[2][0] = target_val_distribution(generator);
	this->world[0][1] = 0.0;
	this->world[1][1] = 0.0;
	this->world[2][1] = 0.0;
	this->world[0][2] = target_val_distribution(generator);
	this->world[1][2] = 0.0;
	this->world[2][2] = target_val_distribution(generator);

	this->current_x = 1;
	this->current_y = 1;

	uniform_int_distribution<int> target_distribution(0, 3);
	for (int t_index = 0; t_index < NUM_TARGETS; t_index++) {
		this->targets.push_back(target_distribution(generator));
	}
	this->curr_index = 0;

	uniform_int_distribution<int> random_factor_distribution(-5, 5);
	// this->random_factor = random_factor_distribution(generator);
	this->random_factor = 0.0;
}

vector<double> Simple::get_observations() {
	vector<double> obs;

	if (this->current_x < 0 || this->current_x > 2
			|| this->current_y < 0 || this->current_y > 2) {
		obs.push_back(-20.0);
	} else {
		obs.push_back(this->world[this->current_x][this->current_y]);
	}

	if (this->curr_index >= NUM_TARGETS) {
		obs.push_back(-1);
	} else {
		obs.push_back(this->targets[this->curr_index]);
	}

	return obs;
}

void Simple::perform_action(int action) {
	switch (action) {
	case ACTION_UP:
		this->current_y++;
		if (this->current_y > 3) {
			this->current_y = 3;
		}
		break;
	case ACTION_RIGHT:
		this->current_x++;
		if (this->current_x > 3) {
			this->current_x = 3;
		}
		break;
	case ACTION_DOWN:
		this->current_y--;
		if (this->current_y < -1) {
			this->current_y = -1;
		}
		break;
	case ACTION_LEFT:
		this->current_x--;
		if (this->current_x < -1) {
			this->current_x = -1;
		}
		break;
	case ACTION_CLICK:
		if (this->curr_index >= NUM_TARGETS) {
			this->world[1][1] -= 3;
		} else {
			switch (this->targets[this->curr_index]) {
			case 0:
				if (this->current_x == 0
						&& this->current_y == 0) {
					this->world[1][1] += 10;

					this->curr_index++;
				} else {
					this->world[1][1] -= 3;
				}
				break;
			case 1:
				if (this->current_x == 2
						&& this->current_y == 0) {
					this->world[1][1] += 10;

					this->curr_index++;
				} else {
					this->world[1][1] -= 3;
				}
				break;
			case 2:
				if (this->current_x == 0
						&& this->current_y == 2) {
					this->world[1][1] += 10;

					this->curr_index++;
				} else {
					this->world[1][1] -= 3;
				}
				break;
			case 3:
				if (this->current_x == 2
						&& this->current_y == 2) {
					this->world[1][1] += 10;

					this->curr_index++;
				} else {
					this->world[1][1] -= 3;
				}
				break;
			}
		}
		break;
	}
}

double Simple::score_result() {
	double score = this->random_factor;
	score += this->world[1][1] / 10.0;
	return score;
}

#if defined(MDEBUG) && MDEBUG
Problem* Simple::copy_and_reset() {
	Simple* new_problem = new Simple();

	new_problem->world = this->world;
	new_problem->world[1][1] = 0.0;

	new_problem->targets = this->targets;

	new_problem->random_factor = this->random_factor;

	return new_problem;
}

Problem* Simple::copy_snapshot() {
	Simple* new_problem = new Simple();

	new_problem->world = this->world;
	new_problem->current_x = this->current_x;
	new_problem->current_y = this->current_y;
	new_problem->targets = this->targets;
	new_problem->curr_index = this->curr_index;
	new_problem->random_factor = this->random_factor;

	return new_problem;
}
#endif /* MDEBUG */

void Simple::print() {
	for (int y_index = 2; y_index >= 0; y_index--) {
		for (int x_index = 0; x_index < 3; x_index++) {
			cout << this->world[x_index][y_index] << " ";
		}
		cout << endl;
	}

	cout << "current_x: " << this->current_x << endl;
	cout << "current_y: " << this->current_y << endl;

	cout << "curr_index: " << curr_index << endl;
}

Problem* TypeSimple::get_problem() {
	return new Simple();
}

int TypeSimple::num_obs() {
	return 2;
}

int TypeSimple::num_possible_actions() {
	return 5;
}
