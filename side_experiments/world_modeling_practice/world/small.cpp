#include "small.h"

#include <iostream>

#include "globals.h"
#include "utilities.h"

using namespace std;

const int ACTION_LEFT = 0;
const int ACTION_RIGHT = 1;
const int ACTION_INCREMENT = 2;

const int WORLD_SIZE = 2;

Small::Small() {
	this->world = vector<double>(WORLD_SIZE);
	uniform_int_distribution<int> noise_distribution(-10, 10);
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		this->world[i_index] = noise_distribution(generator);
	}

	this->curr_index = 0;
}

vector<double> Small::get_observations() {
	vector<double> obs;

	obs.push_back(this->world[this->curr_index]);

	return obs;
}

void Small::perform_action(int action) {
	uniform_int_distribution<int> action_fail_distribution(0, 9);
	if (action_fail_distribution(generator) != 0) {
		switch (action) {
		case ACTION_LEFT:
			this->curr_index--;
			if (this->curr_index < 0) {
				this->curr_index = WORLD_SIZE-1;
			}
			break;
		case ACTION_RIGHT:
			this->curr_index++;
			if (this->curr_index >= WORLD_SIZE) {
				this->curr_index = 0;
			}
			break;
		case ACTION_INCREMENT:
			this->world[this->curr_index]++;
			break;
		}
	}
}

void Small::print() {
	cout << "this->world:";
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		cout << " " << this->world[i_index];
	}
	cout << endl;
	cout << "this->curr_index: " << this->curr_index << endl;
}

Problem* TypeSmall::get_problem() {
	return new Small();
}

int TypeSmall::num_obs() {
	return 1;
}

int TypeSmall::num_possible_actions() {
	return 3;
}
