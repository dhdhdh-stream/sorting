#include "simplest.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int ACTION_LEFT = 0;
const int ACTION_RIGHT = 1;

Simplest::Simplest() {
	this->world = vector<double>(5);

	uniform_int_distribution<int> target_val_distribution(-10, 10);
	uniform_int_distribution<int> truth_val_distribution(-2, 2);
	this->world[0] = target_val_distribution(generator);
	this->world[1] = 0.0;
	this->world[2] = truth_val_distribution(generator);
	this->world[3] = 0.0;
	this->world[4] = target_val_distribution(generator);

	this->curr_index = 0;

	uniform_int_distribution<int> random_factor_distribution(-5, 5);
	this->random_factor = random_factor_distribution(generator);
}

vector<double> Simplest::get_observations() {
	vector<double> obs;

	if (this->curr_index < 0 || this->curr_index > 4) {
		obs.push_back(-20.0);
	} else {
		obs.push_back(this->world[this->curr_index]);
	}

	return obs;
}

void Simplest::perform_action(int action) {
	switch (action) {
	case ACTION_LEFT:
		this->curr_index--;
		if (this->curr_index < -1) {
			this->curr_index = -1;
		}
		break;
	case ACTION_RIGHT:
		this->curr_index++;
		if (this->curr_index > 5) {
			this->curr_index = 5;
		}
		break;
	}
}

double Simplest::score_result() {
	double score = this->random_factor;
	score += this->world[2];
	return score;
}

void Simplest::print() {
	for (int w_index = 0; w_index < 5; w_index++) {
		cout << this->world[w_index] << " ";
	}
	cout << endl;

	cout << "curr_index: " << this->curr_index << endl;
}

Problem* TypeSimplest::get_problem() {
	return new Simplest();
}

int TypeSimplest::num_obs() {
	return 1;
}

int TypeSimplest::num_possible_actions() {
	return 2;
}
