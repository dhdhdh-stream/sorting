#include "problem.h"

#include <algorithm>
#include <iostream>

#include "globals.h"

using namespace std;

Problem::Problem() {
	geometric_distribution<int> length_distribution(0.2);
	int random_length = 2 + length_distribution(generator);

	uniform_int_distribution<int> value_distribution(0, 3);
	for (int i = 0; i < random_length; i++) {
		this->initial_world.push_back(value_distribution(generator));
	}

	this->current_pointer = 0;
	this->current_world = this->initial_world;
}

Problem::~Problem() {
	// do nothing
}

double Problem::get_observation() {
	if (this->current_pointer >= 0 && this->current_pointer < (int)this->current_world.size()) {
		return this->current_world[this->current_pointer];
	} else {
		return -10.0;
	}
}

void Problem::perform_action(Action action) {
	if (action.move == ACTION_LEFT) {
		if (this->current_pointer >= 0) {
			this->current_pointer--;
		}
	} else if (action.move == ACTION_RIGHT) {
		if (this->current_pointer < (int)this->current_world.size()) {
			this->current_pointer++;
		}
	} else if (action.move == ACTION_SWAP) {
		if (this->current_pointer == 0) {
			this->current_world[this->current_pointer] += 1.0;
		} else if (this->current_pointer > 0 && this->current_pointer < (int)this->current_world.size()) {
			this->current_world[this->current_pointer] += 1.0;
			this->current_world[this->current_pointer-1] += -1.0;
		} else if (this->current_pointer == (int)this->current_world.size()) {
			this->current_world[this->current_pointer-1] += -1.0;
		}
	}
}

double Problem::score_result() {
	vector<double> sorted_world = initial_world;
	sort(sorted_world.begin(), sorted_world.end());

	bool correct = true;
	for (int i = 0; i < (int)this->current_world.size(); i++) {
		if (abs(sorted_world[i] - this->current_world[i]) > 0.1) {
			correct = false;
			break;
		}
	}

	if (correct) {
		return 1.0;
	} else {
		return -1.0;
	}
}

void Problem::print() {
	cout << "world";
	for (int i = 0; i < (int)this->current_world.size(); i++) {
		cout << " " << this->current_world[i];
	}
	cout << endl;

	cout << "pointer: " << this->current_pointer << endl;
}
