#include "simpler.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int ACTION_LEFT = 0;
const int ACTION_RIGHT = 1;
const int ACTION_CLICK = 2;

const int NUM_TARGETS = 20;

Simpler::Simpler() {
	this->world = vector<double>(5);

	uniform_int_distribution<int> target_val_distribution(-10, 10);
	this->world[0] = target_val_distribution(generator);
	this->world[1] = 0.0;
	this->world[2] = 0.0;
	this->world[3] = 0.0;
	this->world[4] = target_val_distribution(generator);

	this->curr_index = 2;

	uniform_int_distribution<int> target_distribution(0, 1);
	for (int t_index = 0; t_index < NUM_TARGETS; t_index++) {
		this->targets.push_back(target_distribution(generator));
	}
	this->curr_target_index = 0;

	uniform_int_distribution<int> random_factor_distribution(-5, 5);
	this->random_factor = random_factor_distribution(generator);

	this->timesteps = 0;
}

vector<double> Simpler::get_observations() {
	vector<double> obs;

	if (this->curr_index < 0 || this->curr_index > 4) {
		obs.push_back(-20.0);
	} else {
		obs.push_back(this->world[this->curr_index]);
	}

	if (this->curr_target_index >= (int)this->targets.size()) {
		obs.push_back(-1);
	} else {
		obs.push_back(this->targets[this->curr_target_index]);
	}

	return obs;
}

void Simpler::perform_action(int action) {
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
	case ACTION_CLICK:
		if (this->curr_target_index >= (int)this->targets.size()) {
			this->world[2] -= 2.0;
		} else {
			switch (this->targets[this->curr_target_index]) {
			case 0:
				if (this->curr_index == 0) {
					this->world[2] += 1.0;

					this->curr_target_index++;
				} else {
					this->world[2] -= 2.0;
				}
				break;
			case 1:
				if (this->curr_index == 4) {
					this->world[2] += 1.0;

					this->curr_target_index++;
				} else {
					this->world[2] -= 2.0;
				}
				break;
			}
		}
		break;
	}

	this->timesteps++;
}

double Simpler::score_result() {
	double score = this->random_factor;
	score += this->world[2];
	score -= 0.0001 * this->timesteps;
	return score;
}

void Simpler::print() {
	for (int w_index = 0; w_index < 5; w_index++) {
		cout << this->world[w_index] << " ";
	}
	cout << endl;

	cout << "curr_index: " << this->curr_index << endl;

	cout << "curr_target_index: " << curr_target_index << endl;
	if (curr_target_index >= (int)this->targets.size()) {
		cout << "target: -1" << endl;
	} else {
		cout << "target: " << this->targets[this->curr_target_index] << endl;
	}
}

Problem* TypeSimpler::get_problem() {
	return new Simpler();
}

int TypeSimpler::num_obs() {
	return 2;
}

int TypeSimpler::num_possible_actions() {
	return 3;
}
