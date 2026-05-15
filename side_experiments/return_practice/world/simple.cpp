#include "simple.h"

#include <iostream>

#include "globals.h"

using namespace std;

Simple::Simple() {
	this->current_index = 0;
}

vector<double> Simple::get_observations() {
	vector<double> obs;

	if (this->current_index == 1) {
		obs.push_back(1.0);
	} else {
		obs.push_back(0.0);
	}

	return obs;
}

void Simple::perform_action(int action) {
	switch (action) {
	case 0:
		this->current_index--;
		break;
	case 1:
		this->current_index++;
		break;
	}
}

double Simple::score_result() {
	if (this->current_index == 1) {
		return 1.0;
	} else {
		return -1.0;
	}
}

Problem* TypeSimple::get_problem() {
	return new Simple();
}

int TypeSimple::num_obs() {
	return 1;
}

int TypeSimple::num_possible_actions() {
	return 4;
}
