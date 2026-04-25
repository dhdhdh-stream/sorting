#include "simple.h"

#include <algorithm>
#include <iostream>

#include "globals.h"

using namespace std;

const int WIDTH = 9;
const int HEIGHT = 9;
const int STARTING_X = 4;
const int STARTING_Y = 4;

const int TYPE_EMPTY = 0;
const int TYPE_POINT = 1;
const int TYPE_MINE = 2;

Simple::Simple() {
	this->world = vector<vector<int>>(WIDTH, vector<int>(HEIGHT, 0));
	uniform_int_distribution<int> type_distribution(0, 2);
	for (int x_index = 0; x_index < WIDTH; x_index++) {
		for (int y_index = 0; y_index < HEIGHT; y_index++) {
			this->world[x_index][y_index] = type_distribution(generator);
		}
	}
	this->world[STARTING_X][STARTING_Y] = TYPE_EMPTY;

	this->revealed = vector<vector<bool>>(WIDTH, vector<bool>(HEIGHT, false));
	this->hit = vector<vector<bool>>(WIDTH, vector<bool>(HEIGHT, false));

	this->current_x = STARTING_X;
	this->current_y = STARTING_Y;

	this->hit_mine = false;

	this->hit[this->current_x][this->current_y] = true;
	if (this->world[this->current_x][this->current_y] == TYPE_MINE) {
		this->hit_mine = true;
	}
	for (int x_index = -1; x_index <= 1; x_index++) {
		for (int y_index = -1; y_index <= 1; y_index++) {
			int x_coord = this->current_x + x_index;
			int y_coord = this->current_y + y_index;
			if (x_coord >= 0
					&& x_coord < WIDTH
					&& y_coord >= 0
					&& y_coord < HEIGHT) {
				this->revealed[x_coord][y_coord] = true;
			}
		}
	}
}

double Simple::get_observation_helper(int x, int y) {
	if (x < 0
			|| x > WIDTH-1
			|| y < 0
			|| y > HEIGHT-1) {
		return 5.0;
	} else {
		switch (this->world[x][y]) {
		case TYPE_EMPTY:
			return 0.0;
			break;
		case TYPE_POINT:
			if (this->hit[x][y]) {
				return 1.0;
			} else {
				return 2.0;
			}
			break;
		case TYPE_MINE:
			if (this->hit[x][y]) {
				return -2.0;
			} else {
				return -1.0;
			}
			break;
		}
	}

	return 0.0;		// unreachable
}

vector<double> Simple::get_observations() {
	vector<double> obs;

	for (int x_index = -1; x_index <= 1; x_index++) {
		for (int y_index = -1; y_index <= 1; y_index++) {
			obs.push_back(get_observation_helper(
				this->current_x + x_index,
				this->current_y + y_index
			));
		}
	}

	return obs;
}

void Simple::perform_action(int action) {
	switch (action) {
	case SIMPLE_ACTION_UP:
		this->current_y++;
		if (this->current_y > HEIGHT-1) {
			this->current_y = HEIGHT-1;
		}
		break;
	case SIMPLE_ACTION_RIGHT:
		this->current_x++;
		if (this->current_x > WIDTH-1) {
			this->current_x = WIDTH-1;
		}
		break;
	case SIMPLE_ACTION_DOWN:
		this->current_y--;
		if (this->current_y < 0) {
			this->current_y = 0;
		}
		break;
	case SIMPLE_ACTION_LEFT:
		this->current_x--;
		if (this->current_x < 0) {
			this->current_x = 0;
		}
		break;
	}

	if (!this->hit_mine) {
		this->hit[this->current_x][this->current_y] = true;
		if (this->world[this->current_x][this->current_y] == TYPE_MINE) {
			this->hit_mine = true;
		}
		for (int x_index = -1; x_index <= 1; x_index++) {
			for (int y_index = -1; y_index <= 1; y_index++) {
				int x_coord = this->current_x + x_index;
				int y_coord = this->current_y + y_index;
				if (x_coord >= 0
						&& x_coord < WIDTH
						&& y_coord >= 0
						&& y_coord < HEIGHT) {
					this->revealed[x_coord][y_coord] = true;
				}
			}
		}
	}
}

double Simple::score_result() {
	if (this->hit_mine) {
		return -10.0;
	} else {
		int num_points = 0;
		for (int x_index = 0; x_index < WIDTH; x_index++) {
			for (int y_index = 0; y_index < HEIGHT; y_index++) {
				if (this->world[x_index][y_index] == TYPE_POINT
						&& this->hit[x_index][y_index]) {
					num_points++;
				}
			}
		}
		return num_points;
	}
}

Problem* Simple::copy_and_reset() {
	Simple* new_problem = new Simple();

	new_problem->world = this->world;

	return new_problem;
}

Problem* Simple::copy_snapshot() {
	Simple* new_problem = new Simple();

	new_problem->world = this->world;
	new_problem->revealed = this->revealed;
	new_problem->hit = this->hit;
	new_problem->current_x = this->current_x;
	new_problem->current_y = this->current_y;
	new_problem->hit_mine = this->hit_mine;

	return new_problem;
}

Problem* Simple::create_simulate() {
	Simple* new_problem = new Simple();

	uniform_int_distribution<int> type_distribution(0, 2);
	for (int x_index = 0; x_index < WIDTH; x_index++) {
		for (int y_index = 0; y_index < HEIGHT; y_index++) {
			if (this->revealed[x_index][y_index]) {
				new_problem->world[x_index][y_index] = this->world[x_index][y_index];
			} else {
				new_problem->world[x_index][y_index] = type_distribution(generator);
			}
		}
	}
	new_problem->revealed = this->revealed;
	new_problem->hit = this->hit;
	new_problem->current_x = this->current_x;
	new_problem->current_y = this->current_y;
	new_problem->hit_mine = this->hit_mine;

	return new_problem;
}

void Simple::print() {
	for (int y_index = HEIGHT-1; y_index >= 0; y_index--) {
		for (int x_index = 0; x_index < WIDTH; x_index++) {
			if (this->revealed[x_index][y_index]) {
				cout << get_observation_helper(x_index, y_index);
			} else {
				cout << "-";
			}

			if (x_index == this->current_x
					&& y_index == this->current_y) {
				cout << "<";
			} else {
				cout << " ";
			}
		}
		cout << endl;
	}

	cout << "current_x: " << this->current_x << endl;
	cout << "current_y: " << this->current_y << endl;
}

Problem* TypeSimple::get_problem() {
	return new Simple();
}

int TypeSimple::num_obs() {
	return 9;
}

int TypeSimple::num_possible_actions() {
	return 4;
}
