#include "simple_rl_practice.h"

#include <iostream>

#include "globals.h"

using namespace std;

const int AT_END = -1;
const int OPEN_LEFT = 0;
const int OPEN_MIDDLE = 1;
const int OPEN_RIGHT = 2;

SimpleRLPractice::SimpleRLPractice() {
	uniform_int_distribution<int> end_distribution(0, 2);
	if (end_distribution(generator) == 0) {
		this->curr_target = -1;
	} else {
		uniform_int_distribution<int> target_distribution(0, 2);
		this->curr_target = target_distribution(generator);
	}

	this->score = 0;
}

void SimpleRLPractice::get_observations(vector<double>& obs,
										vector<std::vector<int>>& locations) {
	obs.clear();
	locations.clear();

	obs.push_back((double)this->curr_target);
	locations.push_back(vector<int>());
}

void SimpleRLPractice::perform_action(Action action) {
	if (action.move != ACTION_NOOP) {
		if (this->curr_target == action.move) {
			this->score++;
		} else {
			this->score--;
		}

		if (this->curr_target != AT_END) {
			uniform_int_distribution<int> end_distribution(0, 2);
			if (end_distribution(generator) == 0) {
				this->curr_target = -1;
			} else {
				uniform_int_distribution<int> target_distribution(0, 2);
				this->curr_target = target_distribution(generator);
			}
		}
	}
}

double SimpleRLPractice::score_result(double time_spent) {
	if (this->curr_target != AT_END) {
		this->score--;
	}

	return this->score;
}

vector<int> SimpleRLPractice::get_location() {
	return vector<int>();
}

void SimpleRLPractice::return_to_location(vector<int>& location) {
	// do nothing
}

Problem* SimpleRLPractice::copy_and_reset() {
	return NULL;
}

Problem* SimpleRLPractice::copy_snapshot() {
	return NULL;
}

void SimpleRLPractice::print() {
	// do nothing
}

void SimpleRLPractice::print_obs() {
	// do nothing
}

Problem* TypeSimpleRLPractice::get_problem() {
	return new SimpleRLPractice();
}

int TypeSimpleRLPractice::num_obs() {
	return 1;
}

int TypeSimpleRLPractice::num_possible_actions() {
	return 3;
}

Action TypeSimpleRLPractice::random_action() {
	uniform_int_distribution<int> action_distribution(0, 2);
	return Action(action_distribution(generator));
}

int TypeSimpleRLPractice::num_dimensions() {
	return 0;
}

vector<int> TypeSimpleRLPractice::relative_to_world(
		vector<int>& comparison,
		vector<int>& relative_location) {
	return vector<int>();
}

vector<int> TypeSimpleRLPractice::world_to_relative(
		vector<int>& comparison,
		vector<int>& world_location) {
	return vector<int>();
}
