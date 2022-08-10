#include "problem.h"

#include <algorithm>
#include <iostream>

#include "definitions.h"

using namespace std;

Problem::Problem(vector<double>* observations) {
	int random_length = 2+rand()%9;
	for (int i = 0; i < random_length; i++) {
		this->initial_world.push_back(rand()%11);
	}

	this->current_pointer = 0;
	this->current_world = this->initial_world;

	if (observations != NULL) {
		observations->push_back(get_observation());
	}
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

void Problem::perform_action(Action action,
							 vector<double>* observations,
							 bool save_for_display,
							 vector<Action>* raw_actions) {
	if (action.move == COMPOUND) {
		CompoundAction* compound_action = action_dictionary->actions[action.index];

		CompoundActionNode* curr_node = compound_action->nodes[1];
		while (true) {
			if (curr_node->children_indexes[0] == 0) {
				break;
			}

			Action a = curr_node->children_actions[0];
			perform_action(a,
						   observations,
						   save_for_display,
						   raw_actions);

			curr_node = compound_action->nodes[curr_node->children_indexes[0]];
		}
	} else if (action.move == LOOP) {
		Loop* loop = loop_dictionary->established[action.index];

		loop->pass_through(this,
						   observations,
						   save_for_display,
						   raw_actions);
	} else {
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

		if (observations != NULL) {
			observations->push_back(get_observation());
		}
		if (save_for_display) {
			raw_actions->push_back(action);
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

	cout << "pointer: " << this->current_pointer << endl;
}
