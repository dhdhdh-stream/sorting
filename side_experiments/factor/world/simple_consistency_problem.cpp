#include "simple_consistency_problem.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int ACTION_UP = 0;
const int ACTION_RIGHT = 1;
const int ACTION_DOWN = 2;
const int ACTION_LEFT = 3;
const int ACTION_CLICK = 4;

const int TARGET_UP = 0;
const int TARGET_RIGHT = 1;
const int TARGET_DOWN = 2;
const int TARGET_LEFT = 3;

const int TARGET_LENGTH = 10;

SimpleConsistencyProblem::SimpleConsistencyProblem() {
	this->world = vector<vector<int>>(3, vector<int>(3));
	uniform_int_distribution<int> background_distribution(0, 9);
	for (int x_index = 0; x_index < 3; x_index++) {
		for (int y_index = 0; y_index < 3; y_index++) {
			this->world[x_index][y_index] = background_distribution(generator);
		}
	}
	this->current_x = 1;
	this->current_y = 1;

	this->targets = vector<int>(TARGET_LENGTH);
	uniform_int_distribution<int> target_distribution(0, 3);
	for (int t_index = 0; t_index < TARGET_LENGTH; t_index++) {
		this->targets[t_index] = target_distribution(generator);
	}
	this->curr_target_index = 0;

	this->score = 0.0;
}

vector<double> SimpleConsistencyProblem::get_observations() {
	vector<double> obs;

	obs.push_back(this->world[this->current_x][this->current_y]);

	if (this->curr_target_index >= TARGET_LENGTH) {
		obs.push_back(-1);
	} else {
		obs.push_back(this->targets[this->curr_target_index]);
	}

	return obs;
}

void SimpleConsistencyProblem::perform_action(Action action) {
	switch (action.move) {
	case ACTION_UP:
		this->current_y++;
		if (this->current_y > 2) {
			this->current_y = 2;
		}
		break;
	case ACTION_RIGHT:
		this->current_x++;
		if (this->current_x > 2) {
			this->current_x = 2;
		}
		break;
	case ACTION_DOWN:
		this->current_y--;
		if (this->current_y < 0) {
			this->current_y = 0;
		}
		break;
	case ACTION_LEFT:
		this->current_x--;
		if (this->current_x < 0) {
			this->current_x = 0;
		}
		break;
	case ACTION_CLICK:
		if (this->curr_target_index >= TARGET_LENGTH) {
			this->score -= 1.0;
		} else {
			switch (this->targets[this->curr_target_index]) {
			case TARGET_UP:
				if (this->current_x == 1 && this->current_y == 2) {
					this->score += 1.0;
				} else {
					this->score -= 1.0;
				}
				break;
			case TARGET_RIGHT:
				if (this->current_x == 2 && this->current_y == 1) {
					this->score += 1.0;
				} else {
					this->score -= 1.0;
				}
				break;
			case TARGET_DOWN:
				if (this->current_x == 1 && this->current_y == 0) {
					this->score += 1.0;
				} else {
					this->score -= 1.0;
				}
				break;
			case TARGET_LEFT:
				if (this->current_x == 0 && this->current_y == 1) {
					this->score += 1.0;
				} else {
					this->score -= 1.0;
				}
				break;
			}

			this->curr_target_index++;
		}
		break;
	}
}

double SimpleConsistencyProblem::score_result() {
	return score;
}

#if defined(MDEBUG) && MDEBUG
Problem* SimpleConsistencyProblem::copy_and_reset() {
	SimpleConsistencyProblem* new_problem = new SimpleConsistencyProblem();

	new_problem->world = this->world;

	new_problem->targets = this->targets;

	return new_problem;
}

Problem* SimpleConsistencyProblem::copy_snapshot() {
	SimpleConsistencyProblem* new_problem = new SimpleConsistencyProblem();

	new_problem->world = this->world;
	new_problem->current_x = this->current_x;
	new_problem->current_y = this->current_y;

	new_problem->targets = this->targets;
	new_problem->curr_target_index = this->curr_target_index;

	new_problem->score = this->score;

	return new_problem;
}
#endif /* MDEBUG */

void SimpleConsistencyProblem::print() {
	for (int y_index = 3-1; y_index >= 0; y_index--) {
		for (int x_index = 0; x_index < 3; x_index++) {
			cout << this->world[x_index][y_index] << " ";
		}
		cout << endl;
	}

	cout << "current_x: " << this->current_x << endl;
	cout << "current_y: " << this->current_y << endl;

	cout << "targets:";
	for (int t_index = 0; t_index < (int)this->targets.size(); t_index++) {
		cout << " " << this->targets[t_index];
	}
	cout << endl;
	cout << "curr_target_index: " << curr_target_index << endl;

	cout << "this->score: " << this->score << endl;
}

Problem* TypeSimpleConsistencyProblem::get_problem() {
	return new SimpleConsistencyProblem();
}

int TypeSimpleConsistencyProblem::num_obs() {
	return 2;
}

int TypeSimpleConsistencyProblem::num_possible_actions() {
	return 5;
}

Action TypeSimpleConsistencyProblem::random_action() {
	uniform_int_distribution<int> action_distribution(0, 4);
	return Action(action_distribution(generator));
}
