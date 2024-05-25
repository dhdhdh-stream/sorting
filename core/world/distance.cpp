#include "distance.h"

#include <cmath>

#include "globals.h"

using namespace std;

const int WIDTH = 5;
const int HEIGHT = 5;

Distance::Distance() {
	uniform_int_distribution<int> x_distribution(0, WIDTH-1);
	uniform_int_distribution<int> y_distribution(0, HEIGHT-1);

	this->object_start_x = x_distribution(generator);
	this->object_start_y = y_distribution(generator);

	this->object_end_x = x_distribution(generator);
	this->object_end_y = y_distribution(generator);

	this->current_x = x_distribution(generator);
	this->current_y = y_distribution(generator);

	this->is_start = true;
}

int Distance::num_obs() {
	return 9;
}

int Distance::num_possible_actions() {
	/**
	 * - don't include DISTANCE_ACTION_SHUFFLE
	 */
	return 4;
}

Action Distance::random_action() {
	uniform_int_distribution<int> action_distribution(0, 3);
	return Action(action_distribution(generator));
}

double Distance::get_observation_helper(int x, int y) {
	if (x < 0
			|| x > WIDTH-1
			|| y < 0
			|| y > HEIGHT-1) {
		return -1.0;
	} else {
		if (this->is_start) {
			if (x == this->object_start_x
					&& y == this->object_start_y) {
				return 1.0;
			} else {
				return 0.0;
			}
		} else {
			if (x == this->object_end_x
					&& y == this->object_end_y) {
				return 1.0;
			} else {
				return 0.0;
			}
		}
	}
}

vector<double> Distance::get_observations() {
	vector<double> obs;

	obs.push_back(get_observation_helper(this->current_x, this->current_y));

	obs.push_back(get_observation_helper(this->current_x-1, this->current_y+1));
	obs.push_back(get_observation_helper(this->current_x, this->current_y+1));
	obs.push_back(get_observation_helper(this->current_x+1, this->current_y+1));
	obs.push_back(get_observation_helper(this->current_x+1, this->current_y));
	obs.push_back(get_observation_helper(this->current_x+1, this->current_y-1));
	obs.push_back(get_observation_helper(this->current_x, this->current_y-1));
	obs.push_back(get_observation_helper(this->current_x-1, this->current_y-1));
	obs.push_back(get_observation_helper(this->current_x-1, this->current_y));

	return obs;
}

void Distance::perform_action(Action action) {
	switch (action.move) {
	case DISTANCE_ACTION_UP:
		if (this->current_y < HEIGHT-1) {
			this->current_y++;
		}
		break;
	case DISTANCE_ACTION_RIGHT:
		if (this->current_x < WIDTH-1) {
			this->current_x++;
		}
		break;
	case DISTANCE_ACTION_DOWN:
		if (this->current_y > 0) {
			this->current_y--;
		}
		break;
	case DISTANCE_ACTION_LEFT:
		if (this->current_x > 0) {
			this->current_x--;
		}
		break;
	case DISTANCE_ACTION_SEQUENCE:
		{
			uniform_int_distribution<int> x_distribution(0, WIDTH-1);
			uniform_int_distribution<int> y_distribution(0, HEIGHT-1);

			this->current_x = x_distribution(generator);
			this->current_y = y_distribution(generator);

			this->is_start = false;
		}
		break;
	}
}

double Distance::score_result(int num_decisions) {
	double x_distance = this->object_end_x - this->object_start_x;
	double y_distance = this->object_end_y - this->object_start_y;

	return sqrt(x_distance * x_distance + y_distance * y_distance);
}

Problem* Distance::copy_snapshot() {
	Distance* new_distance = new Distance();

	new_distance->object_start_x = this->object_start_x;
	new_distance->object_start_y = this->object_start_y;

	new_distance->object_end_x = this->object_end_x;
	new_distance->object_end_y = this->object_end_y;

	new_distance->current_x = this->current_x;
	new_distance->current_y = this->current_y;

	new_distance->is_start = this->is_start;

	return new_distance;
}

void Distance::print() {

}
