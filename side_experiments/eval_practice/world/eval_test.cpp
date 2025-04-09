#include "eval_test.h"

#include <iostream>

#include "globals.h"

using namespace std;

EvalTest::EvalTest() {
	uniform_int_distribution<int> distribution(-10, 10);
	this->world = vector<vector<int>>(3, vector<int>(3));
	this->world[0][0] = distribution(generator);
	this->world[0][1] = 0;
	this->world[0][2] = distribution(generator);
	this->world[1][0] = 0;
	this->world[1][1] = distribution(generator);
	this->world[1][2] = 0;
	this->world[2][0] = distribution(generator);
	this->world[2][1] = 0;
	this->world[2][2] = distribution(generator);

	this->current_x = 1;
	this->current_y = 1;
}

vector<double> EvalTest::get_observations() {
	vector<double> obs;

	if (this->current_x < 0
			|| this->current_x > 2
			|| this->current_y < 0
			|| this->current_y > 2) {
		obs.push_back(-20.0);
	} else {
		obs.push_back(this->world[this->current_x][this->current_y]);
	}

	return obs;
}

void EvalTest::perform_action(Action action) {
	switch (action.move) {
	case EVAL_TEST_ACTION_UP:
		this->current_y++;
		if (this->current_y > 3) {
			this->current_y = 3;
		}
		break;
	case EVAL_TEST_ACTION_RIGHT:
		this->current_x++;
		if (this->current_x > 3) {
			this->current_x = 3;
		}
		break;
	case EVAL_TEST_ACTION_DOWN:
		this->current_y--;
		if (this->current_y < -1) {
			this->current_y = -1;
		}
		break;
	case EVAL_TEST_ACTION_LEFT:
		this->current_x--;
		if (this->current_x < -1) {
			this->current_x = -1;
		}
		break;
	}
}

double EvalTest::score_result() {
	return this->world[1][1];
}

void EvalTest::print() {
	for (int y_index = 2; y_index >= 0; y_index--) {
		for (int x_index = 0; x_index < 2; x_index++) {
			cout << this->world[x_index][y_index];

			if (x_index == this->current_x
					&& y_index == this->current_y) {
				cout << "<";
			} else {
				cout << " ";
			}
		}
		cout << endl;
	}

	cout << "current_x: " << this->current_x << endl;
	cout << "current_y: " << this->current_y << endl;
}

Problem* TypeEvalTest::get_problem() {
	return new EvalTest();
}

int TypeEvalTest::num_obs() {
	return 1;
}

int TypeEvalTest::num_possible_actions() {
	return 4;
}

Action TypeEvalTest::random_action() {
	uniform_int_distribution<int> action_distribution(0, 3);
	return Action(action_distribution(generator));
}
