#include "test_indirect.h"

#include <iostream>

#include "globals.h"

using namespace std;

TestIndirect::TestIndirect() {
	uniform_int_distribution<int> context_distribution(0, 9);
	if (context_distribution(generator) == 0) {
		this->curr_context = 1;
	} else {
		this->curr_context = 0;
	}
	this->current_index = 0;
}

vector<double> TestIndirect::get_observations() {
	vector<double> obs;

	obs.push_back(this->curr_context);

	return obs;
}

void TestIndirect::perform_action(int action) {
	switch (action) {
	case 0:
		if (this->curr_context == 1) {
			this->current_index++;
		} else {
			this->current_index--;
		}
		break;
	case 1:
		if (this->curr_context == 1) {
			this->current_index--;
		} else {
			this->current_index++;
		}
		break;
	}

	uniform_int_distribution<int> context_distribution(0, 9);
	if (context_distribution(generator) == 0) {
		this->curr_context = 1;
	} else {
		this->curr_context = 0;
	}
}

double TestIndirect::score_result() {
	if (this->current_index == 1) {
		return 1.0;
	} else {
		return -1.0;
	}
}

Problem* TypeTestIndirect::get_problem() {
	return new TestIndirect();
}

int TypeTestIndirect::num_obs() {
	return 1;
}

int TypeTestIndirect::num_possible_actions() {
	return 4;
}
