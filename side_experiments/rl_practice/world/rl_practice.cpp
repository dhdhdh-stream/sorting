#include "rl_practice.h"

#include "globals.h"

using namespace std;

const int OPEN_LEFT = 0;
const int OPEN_MIDDLE = 1;
const int OPEN_RIGHT = 2;

RLPractice::RLPractice() {
	uniform_int_distribution<int> door_distribution(0, 2);
	this->open_door = door_distribution(generator);

	uniform_int_distribution<int> at_end_distribution(0, 2);
	if (at_end_distribution(generator) == 0) {
		this->at_end = true;
	} else {
		this->at_end = false;
	}

	uniform_int_distribution<int> needed_distribution(0, 1);
	if (needed_distribution(generator) == 0) {
		this->left_needed = true;
	} else {
		this->left_needed = false;
	}

	this->action_performed = false;
	this->past_end = false;
}

void RLPractice::get_observations(vector<double>& obs,
								  vector<std::vector<int>>& locations) {
	if (this->at_end) {
		if (this->left_needed) {
			obs.push_back(3.0);
			locations.push_back(vector<int>());
		} else {
			obs.push_back(4.0);
			locations.push_back(vector<int>());
		}
	} else {
		obs.push_back((double)this->open_door);
		locations.push_back(vector<int>());
	}
}

void RLPractice::perform_action(Action action) {
	switch (action.move) {
	case RL_PRACTICE_MOVE_LEFT:
		if (this->at_end) {
			uniform_int_distribution<int> door_distribution(0, 2);
			this->open_door = door_distribution(generator);
			this->at_end = false;

			this->past_end = true;
		} else if (this->open_door == OPEN_LEFT) {
			uniform_int_distribution<int> door_distribution(0, 2);
			this->open_door = door_distribution(generator);

			if (!this->past_end) {
				uniform_int_distribution<int> at_end_distribution(0, 2);
				if (at_end_distribution(generator) == 0) {
					this->at_end = true;
				} else {
					this->at_end = false;
				}
			}
		}
		break;
	case RL_PRACTICE_MOVE_MIDDLE:
		if (this->at_end) {
			uniform_int_distribution<int> door_distribution(0, 2);
			this->open_door = door_distribution(generator);
			this->at_end = false;

			this->past_end = true;
		} else if (this->open_door == OPEN_MIDDLE) {
			uniform_int_distribution<int> door_distribution(0, 2);
			this->open_door = door_distribution(generator);

			if (!this->past_end) {
				uniform_int_distribution<int> at_end_distribution(0, 2);
				if (at_end_distribution(generator) == 0) {
					this->at_end = true;
				} else {
					this->at_end = false;
				}
			}
		}
		break;
	case RL_PRACTICE_MOVE_RIGHT:
		if (this->at_end) {
			uniform_int_distribution<int> door_distribution(0, 2);
			this->open_door = door_distribution(generator);
			this->at_end = false;

			this->past_end = true;
		} else if (this->open_door == OPEN_RIGHT) {
			uniform_int_distribution<int> door_distribution(0, 2);
			this->open_door = door_distribution(generator);

			if (!this->past_end) {
				uniform_int_distribution<int> at_end_distribution(0, 2);
				if (at_end_distribution(generator) == 0) {
					this->at_end = true;
				} else {
					this->at_end = false;
				}
			}
		}
		break;
	case RL_PRACTICE_PERFORM_LEFT:
		if (this->at_end) {
			if (this->left_needed) {
				this->action_performed = true;
			} else {
				this->action_performed = false;
			}
		}
		break;
	case RL_PRACTICE_PERFORM_RIGHT:
		if (this->at_end) {
			if (!this->left_needed) {
				this->action_performed = true;
			} else {
				this->action_performed = false;
			}
		}
		break;
	}
}

double RLPractice::score_result(double time_spent) {
	double score = 0.0;
	if (this->action_performed && this->at_end) {
		score += 1.0;
	}

	score -= time_spent * 0.005;

	return score;
}

vector<int> RLPractice::get_location() {
	return vector<int>();
}

void RLPractice::return_to_location(vector<int>& location) {
	// do nothing
}

Problem* RLPractice::copy_and_reset() {
	return NULL;
}

Problem* RLPractice::copy_snapshot() {
	return NULL;
}

void RLPractice::print() {
	// do nothing
}

void RLPractice::print_obs() {
	// do nothing
}

Problem* TypeRLPractice::get_problem() {
	return new RLPractice();
}

int TypeRLPractice::num_obs() {
	return 1;
}

int TypeRLPractice::num_possible_actions() {
	return 5;
}

Action TypeRLPractice::random_action() {
	uniform_int_distribution<int> action_distribution(0, 4);
	return Action(action_distribution(generator));
}

int TypeRLPractice::num_dimensions() {
	return 0;
}

vector<int> TypeRLPractice::relative_to_world(
		vector<int>& comparison,
		vector<int>& relative_location) {
	return vector<int>();
}

vector<int> TypeRLPractice::world_to_relative(
		vector<int>& comparison,
		vector<int>& world_location) {
	return vector<int>();
}
