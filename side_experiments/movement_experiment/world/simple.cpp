#include "simple.h"

#include <algorithm>
#include <iostream>

#include "globals.h"

using namespace std;

const int WORLD_SIZE = 10;

Simple::Simple() {
	this->world = vector<int>(WORLD_SIZE);
	uniform_int_distribution<int> distribution(0, 9);
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		this->world[i_index] = distribution(generator);
	}

	this->current_index = 0;
}

vector<double> Simple::get_observations() {
	vector<double> obs;

	for (int i_index = -2; i_index <= 2; i_index++) {
		int index = this->current_index + i_index;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		obs.push_back(this->world[index]);
	}

	return obs;
}

void Simple::perform_action(int action) {
	switch (action) {
	case SIMPLE_ACTION_LEFT:
		this->current_index--;
		break;
	case SIMPLE_ACTION_RIGHT:
		this->current_index++;
		break;
	}

	// uniform_int_distribution<int> uncertainty_distribution(0, 9);
	// switch (uncertainty_distribution(generator)) {
	// case 0:
	// 	this->current_index--;
	// 	break;
	// case 1:
	// 	this->current_index++;
	// 	break;
	// }

	if (this->current_index < 0) {
		this->current_index += WORLD_SIZE;
	}
	if (this->current_index >= WORLD_SIZE) {
		this->current_index -= WORLD_SIZE;
	}
}

void Simple::print() {
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		cout << this->world[i_index] << " ";
	}
	cout << endl;

	cout << "this->current_index: " << this->current_index << endl;
}

Problem* TypeSimple::get_problem() {
	return new Simple();
}

int TypeSimple::num_obs() {
	return 5;
}

int TypeSimple::num_possible_actions() {
	return 2;
}
