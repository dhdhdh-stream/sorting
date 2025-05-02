#include "two_dimensional.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int X_SIZE = 4;
const int Y_SIZE = 4;

TwoDimensional::TwoDimensional() {
	this->world = vector<vector<double>>(X_SIZE, vector<double>(Y_SIZE));
	for (int i_index = 0; i_index < X_SIZE; i_index++) {
		for (int j_index = 0; j_index < Y_SIZE; j_index++) {
			this->world[i_index][j_index] = i_index * Y_SIZE + j_index;
		}
	}
	this->x_index = 0;
	this->y_index = 0;
}

double TwoDimensional::get_observation() {
	if (this->x_index < 0
			|| this->x_index > X_SIZE-1
			|| this->y_index < 0
			|| this->y_index > Y_SIZE-1) {
		return -20.0;
	} else {
		return this->world[this->x_index][this->y_index];
	}
}

void TwoDimensional::perform_action(Action action) {
	uniform_int_distribution<int> zero_distribution(1, 40);
	if (zero_distribution(generator) != 0) {
		uniform_int_distribution<int> double_distribution(1, 39);
		bool is_double = double_distribution(generator) == 0;
		switch (action.move) {
		case TWO_DIMENSIONAL_ACTION_UP:
			if (is_double) {
				y_index += 2;
			} else {
				y_index++;
			}
			if (y_index > Y_SIZE) {
				y_index = Y_SIZE;
			}
			break;
		case TWO_DIMENSIONAL_ACTION_RIGHT:
			if (is_double) {
				x_index += 2;
			} else {
				x_index++;
			}
			if (x_index > X_SIZE) {
				x_index = X_SIZE;
			}
			break;
		case TWO_DIMENSIONAL_ACTION_DOWN:
			if (is_double) {
				y_index -= 2;
			} else {
				y_index--;
			}
			if (y_index < -1) {
				y_index = -1;
			}
			break;
		case TWO_DIMENSIONAL_ACTION_LEFT:
			if (is_double) {
				x_index -= 2;
			} else {
				x_index--;
			}
			if (x_index < -1) {
				x_index = -1;
			}
			break;
		}
	}
}

void TwoDimensional::print() {
	cout << "this->x_index: " << this->x_index << endl;
	cout << "this->y_index: " << this->y_index << endl;
}

Problem* TypeTwoDimensional::get_problem() {
	return new TwoDimensional();
}

int TypeTwoDimensional::num_obs() {
	return 1;
}

int TypeTwoDimensional::num_possible_actions() {
	return 4;
}
