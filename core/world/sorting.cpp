#include "sorting.h"

#include <algorithm>
#include <iostream>

#include "globals.h"

using namespace std;

Sorting::Sorting() {
	geometric_distribution<int> length_distribution(0.2);
	int random_length = 1 + length_distribution(generator);

	uniform_int_distribution<int> value_distribution(0, 3);
	for (int i = 0; i < random_length; i++) {
		this->initial_world.push_back(value_distribution(generator));
	}

	this->current_pointer = 0;
	this->current_world = this->initial_world;
}

vector<double> Sorting::get_observations() {
	if (this->current_pointer >= 0 && this->current_pointer < (int)this->current_world.size()) {
		return vector<double>{this->current_world[this->current_pointer]};
	} else {
		return vector<double>{-10.0};
	}
}

void Sorting::perform_action(Action action) {
	if (action.move == SORTING_ACTION_LEFT) {
		if (this->current_pointer >= 0) {
			this->current_pointer--;
		}
	} else if (action.move == SORTING_ACTION_RIGHT) {
		if (this->current_pointer < (int)this->current_world.size()) {
			this->current_pointer++;
		}
	} else if (action.move == SORTING_ACTION_SWAP) {
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

double Sorting::score_result(int num_decisions) {
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

Problem* Sorting::copy_and_reset() {
	Sorting* new_problem = new Sorting();

	new_problem->initial_world = this->initial_world;

	new_problem->current_world = this->initial_world;
	new_problem->current_pointer = 0;

	return new_problem;
}

Problem* Sorting::copy_snapshot() {
	Sorting* new_problem = new Sorting();

	new_problem->initial_world = this->initial_world;

	new_problem->current_world = this->current_world;
	new_problem->current_pointer = this->current_pointer;

	return new_problem;
}

void Sorting::print() {
	cout << "initial world:";
	for (int i = 0; i < (int)this->initial_world.size(); i++) {
		cout << " " << this->initial_world[i];
	}
	cout << endl;

	cout << "current world:";
	for (int i = 0; i < (int)this->current_world.size(); i++) {
		cout << " " << this->current_world[i];
	}
	cout << endl;

	cout << "pointer: " << this->current_pointer << endl;
}

Problem* TypeSorting::get_problem() {
	return new Sorting();
}

int TypeSorting::num_obs() {
	return 1;
}

int TypeSorting::num_possible_actions() {
	return 3;
}

Action TypeSorting::random_action() {
	uniform_int_distribution<int> action_distribution(0, 2);
	return Action(action_distribution(generator));
}
