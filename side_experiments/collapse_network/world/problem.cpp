#include "problem.h"

#include <algorithm>
#include <iostream>

using namespace std;

Problem::Problem() {
	int random_length = 2+rand()%9;
	for (int i = 0; i < random_length; i++) {
		this->initial_world.push_back(rand()%11);
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
		return -100.0;
	}
}

void Problem::perform_action(Action action) {
	if (this->current_pointer >= 0 && this->current_pointer < (int)this->current_world.size()) {
		this->current_world[this->current_pointer] += action.write;
	}

	if (action.move == LEFT) {
		if (this->current_pointer >= 0) {
			this->current_pointer--;
		}
	} else if (action.move == RIGHT) {
		if (this->current_pointer < (int)this->current_world.size()) {
			this->current_pointer++;
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
		return 0.0;
	}
}

void Problem::print() {
	cout << "world";
	for (int i = 0; i < (int)this->current_world.size(); i++) {
		cout << " " << this->current_world[i];
	}
	cout << endl;

	// cout << "pointer: " << this->current_pointer << endl;
}
