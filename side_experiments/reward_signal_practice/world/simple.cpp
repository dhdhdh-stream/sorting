#include "simple.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int ACTION_LEFT = 0;
const int ACTION_RIGHT = 1;

const int WORLD_SIZE = 9;

Simple::Simple() {
	this->world = vector<int>(9);

	uniform_int_distribution<int> val_distribution(-10, 10);
	for (int w_index = 0; w_index < WORLD_SIZE; w_index++) {
		this->world[w_index] = val_distribution(generator);
	}

	this->world[WORLD_SIZE / 2 - 1] = 0.0;
	this->world[WORLD_SIZE / 2 + 1] = 0.0;

	this->current_index = 0;

	uniform_int_distribution<int> noise_distribution(-10, 10);
	this->score = this->world[WORLD_SIZE / 2] + noise_distribution(generator);
}

vector<double> Simple::get_observations() {
	vector<double> obs;

	if (this->current_index < 0 || this->current_index > WORLD_SIZE-1) {
		obs.push_back(-20.0);
	} else {
		obs.push_back(this->world[this->current_index]);
	}

	return obs;
}

void Simple::perform_action(int action) {
	switch (action) {
	case ACTION_LEFT:
		this->current_index--;
		if (this->current_index < -1) {
			this->current_index = -1;
		}
		break;
	case ACTION_RIGHT:
		this->current_index++;
		if (this->current_index > WORLD_SIZE) {
			this->current_index = WORLD_SIZE;
		}
		break;
	}
}

double Simple::score_result() {
	return this->score;
}

void Simple::print() {
	for (int w_index = 0; w_index < WORLD_SIZE; w_index++) {
		cout << this->world[w_index] << " ";
	}
	cout << endl;

	cout << "this->current_index: " << this->current_index << endl;
}

Problem* TypeSimple::get_problem() {
	return new Simple();
}

int TypeSimple::num_obs() {
	return 1;
}

int TypeSimple::num_possible_actions() {
	return 2;
}
