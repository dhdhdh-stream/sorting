#include "simple_problem.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int WORLD_SIZE = 5;

SimpleProblem::SimpleProblem() {
	this->clicked = vector<bool>(WORLD_SIZE, false);

	uniform_int_distribution<int> target_distribution(0, WORLD_SIZE-1);
	this->target_index = target_distribution(generator);
	this->target_index = target_distribution(generator);

	this->current_index = 0;
}

vector<double> SimpleProblem::get_observations() {
	vector<double> obs;

	if (this->current_index < 0
			|| this->current_index > WORLD_SIZE-1) {
		obs.push_back(-20.0);
	} else if (this->current_index == this->target_index) {
		if (this->clicked[this->current_index]) {
			obs.push_back(10.0);
		} else {
			obs.push_back(1.0);
		}
	} else {
		if (this->clicked[this->current_index]) {
			obs.push_back(-10.0);
		} else {
			obs.push_back(-1.0);
		}
	}

	return obs;
}

void SimpleProblem::perform_action(Action action) {
	switch (action.move) {
	case SIMPLE_ACTION_LEFT:
		this->current_index--;
		if (this->current_index < -1) {
			this->current_index = -1;
		}
		break;
	case SIMPLE_ACTION_RIGHT:
		this->current_index++;
		if (this->current_index > WORLD_SIZE) {
			this->current_index = WORLD_SIZE;
		}
		break;
	case SIMPLE_ACTION_CLICK:
		if (this->current_index >= 0 && this->current_index < WORLD_SIZE) {
			this->clicked[this->current_index] = !this->clicked[this->current_index];
		}
		break;
	}
}

double SimpleProblem::score_result() {
	bool has_clicked = false;
	bool has_clicked_incorrect = false;
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		if (this->clicked[i_index]) {
			has_clicked = true;
			if (i_index != this->target_index) {
				has_clicked_incorrect = true;
				break;
			}
		}
	}

	if (has_clicked) {
		if (has_clicked_incorrect) {
			return -1.0;
		} else {
			return 1.0;
		}
	} else {
		return 0.0;
	}
}

#if defined(MDEBUG) && MDEBUG
Problem* SimpleProblem::copy_and_reset() {
	SimpleProblem* new_problem = new SimpleProblem();

	new_problem->target_index = this->target_index;

	return new_problem;
}

Problem* SimpleProblem::copy_snapshot() {
	SimpleProblem* new_problem = new SimpleProblem();

	new_problem->clicked = this->clicked;
	new_problem->target_index = this->target_index;
	new_problem->current_index = this->current_index;

	return new_problem;
}
#endif /* MDEBUG */

void SimpleProblem::print() {
	cout << "clicked:";
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		cout << " " << this->clicked[i_index];
	}
	cout << endl;
	cout << "this->target_index: " << this->target_index << endl;
	cout << "this->current_index: " << this->current_index << endl;
}

Problem* TypeSimpleProblem::get_problem() {
	return new SimpleProblem();
}

int TypeSimpleProblem::num_obs() {
	return 1;
}

int TypeSimpleProblem::num_possible_actions() {
	return 3;
}
