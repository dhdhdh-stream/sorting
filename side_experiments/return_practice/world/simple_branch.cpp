#include "simple_branch.h"

#include <iostream>

#include "globals.h"

using namespace std;

SimpleBranch::SimpleBranch() {
	uniform_int_distribution<int> is_left_distribution(0, 1);
	if (is_left_distribution(generator) == 0) {
		this->is_left = true;
	} else {
		this->is_left = false;
	}

	this->current_index = 0;
}

vector<double> SimpleBranch::get_observations() {
	vector<double> obs;

	if (this->is_left) {
		obs.push_back(1.0);
	} else {
		obs.push_back(-1.0);
	}

	return obs;
}

void SimpleBranch::perform_action(int action) {
	switch (action) {
	case 0:
		this->current_index--;
		break;
	case 1:
		this->current_index++;
		break;
	}
}

double SimpleBranch::score_result() {
	if (this->is_left) {
		if (this->current_index == -1) {
			return 1.0;
		} else {
			return -1.0;
		}
	} else {
		if (this->current_index == 1) {
			return 1.0;
		} else {
			return -1.0;
		}
	}
}

Problem* TypeSimpleBranch::get_problem() {
	return new SimpleBranch();
}

int TypeSimpleBranch::num_obs() {
	return 1;
}

int TypeSimpleBranch::num_possible_actions() {
	return 4;
}
