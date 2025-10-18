#include "problem.h"

#include "globals.h"

using namespace std;

Problem::Problem() {
	this->current_x = 0;
	this->current_y = 0;
}

vector<double> Problem::get_observations() {
	vector<double> obs;

	obs.push_back(this->current_x);
	obs.push_back(this->current_y);

	return obs;
}

bool Problem::perform_action(int action) {
	switch (action) {
	case ACTION_UP:
		this->current_y++;
		break;
	case ACTION_RIGHT:
		this->current_x++;
		break;
	case ACTION_DOWN:
		this->current_y--;
		break;
	case ACTION_LEFT:
		this->current_x--;
		break;
	}

	if (this->current_x == 4
			&& this->current_y == 4) {
		return true;
	} else {
		return false;
	}
}

void get_existing_solution(vector<int>& actions) {
	int current_x = 0;
	int current_y = 0;

	uniform_int_distribution<int> vert_distribution(0, 1);
	uniform_int_distribution<int> distribution(0, 19);
	while (true) {
		if (vert_distribution(generator) == 0) {
			if (current_y > 4) {
				if (distribution(generator) < 3) {
					current_y++;
					actions.push_back(ACTION_UP);
				} else {
					current_y--;
					actions.push_back(ACTION_DOWN);
				}
			} else if (current_y < 4) {
				if (distribution(generator) < 17) {
					current_y++;
					actions.push_back(ACTION_UP);
				} else {
					current_y--;
					actions.push_back(ACTION_DOWN);
				}
			} else {
				if (distribution(generator) < 10) {
					current_y++;
					actions.push_back(ACTION_UP);
				} else {
					current_y--;
					actions.push_back(ACTION_DOWN);
				}
			}
		} else {
			if (current_x > 4) {
				if (distribution(generator) < 3) {
					current_x++;
					actions.push_back(ACTION_RIGHT);
				} else {
					current_x--;
					actions.push_back(ACTION_LEFT);
				}
			} else if (current_x < 4) {
				if (distribution(generator) < 17) {
					current_x++;
					actions.push_back(ACTION_RIGHT);
				} else {
					current_x--;
					actions.push_back(ACTION_LEFT);
				}
			} else {
				if (distribution(generator) < 10) {
					current_x++;
					actions.push_back(ACTION_RIGHT);
				} else {
					current_x--;
					actions.push_back(ACTION_LEFT);
				}
			}
		}

		if (current_x == 4
				&& current_y == 4) {
			break;
		}
	}
}
