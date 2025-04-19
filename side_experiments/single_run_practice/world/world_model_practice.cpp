#include "world_mode_practice.h"

using namespace std;

const int WORLD_SIZE = 5;

WorldModelPractice::WorldModelPractice() {
	uniform_int_distribution<int> distribution(0, 10);
	this->world = vector<int>(WORLD_SIZE);
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		this->world[i_index] = distribution(generator);
	}
	this->current_index = 2;

	this->perform_action = false;
}

vector<double> WorldModelPractice::get_observations() {
	vector<double> obs;

	if (this->current_index < 0 || this->current_index > WORLD_SIZE-1) {
		obs.push_back(-10.0);
	} else {
		obs.push_back(this->world[this->current_index]);
	}

	return obs;
}

void WorldModelPractice::perform_action(Action action) {
	switch (action.move) {
	case WORLD_MODEL_ACTION_LEFT:
		this->current_index--;
		if (this->current_index < -1) {
			this->current_index = -1;
		}
		break;
	case WORLD_MODEL_ACTION_RIGHT:
		this->current_index++;
		if (this->current_index > WORLD_SIZE) {
			this->current_index = WORLD_SIZE;
		}
		break;
	case WORLD_MODEL_ACTION_ACTION:
		if (!this->perform_action) {
			if (this->current_index >= 0 && this->current_index <= WORLD_SIZE-1) {
				this->world[this->current_index] = 20.0;

				this->perform_action = true;
			}
		}
	}
}

void WorldModelPractice::print() {
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		cout << this->world[i_index] << " ";
	}
	cout << endl;

	cout << "this->current_index: " << this->current_index << endl;
}

Problem* TypeWorldModelPractice::get_problem() {
	return new WorldModelPractice();
}

int TypeWorldModelPractice::num_obs() {
	return 1;
}

int TypeWorldModelPractice::num_possible_actions() {
	return 3;
}
