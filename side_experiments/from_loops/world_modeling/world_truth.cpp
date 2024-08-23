#include "world_truth.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

WorldTruth::WorldTruth() {
	uniform_int_distribution<int> world_size_distribution(5, 7);
	this->world_size = world_size_distribution(generator);

	uniform_int_distribution<int> starting_distribution(0, this->world_size-1);
	this->curr_x = starting_distribution(generator);
	this->curr_y = starting_distribution(generator);

	uniform_int_distribution<int> obj_starting_distribution(1, this->world_size-2);
	this->obj_x = obj_starting_distribution(generator);
	this->obj_y = obj_starting_distribution(generator);

	uniform_int_distribution<int> vel_distribution(-1, 1);
	this->obj_x_vel = vel_distribution(generator);
	this->obj_y_vel = vel_distribution(generator);

	this->action_queue.push_back(ACTION_NOOP);
	this->action_queue.push_back(ACTION_NOOP);
}

double WorldTruth::get_obs() {
	if (this->curr_x == this->obj_x
			&& this->curr_y == this->obj_y) {
		return 2.0;
	} else if (this->curr_x == 0
			|| this->curr_x == this->world_size-1
			|| this->curr_y == 0
			|| this->curr_y == this->world_size-1) {
		return 1.0;
	} else {
		return 0.0;
	}
}

void WorldTruth::move(int action) {
	uniform_int_distribution<int> stay_distribution(0, 9);
	switch (this->action_queue[0]) {
	case ACTION_LEFT:
		if (this->curr_x > 0) {
			if (stay_distribution(generator) != 0) {
				this->curr_x--;
			}
		}
		break;
	case ACTION_UP:
		if (this->curr_y < this->world_size-1) {
			if (stay_distribution(generator) != 0) {
				this->curr_y++;
			}
		}
		break;
	case ACTION_RIGHT:
		if (this->curr_x < this->world_size-1) {
			if (stay_distribution(generator) != 0) {
				this->curr_x++;
			}
		}
		break;
	case ACTION_DOWN:
		if (this->curr_y > 0) {
			if (stay_distribution(generator) != 0) {
				this->curr_y--;
			}
		}
		break;
	}

	if (this->obj_x_vel == 1) {
		if (this->obj_x < this->world_size-2) {
			this->obj_x++;
		} else {
			this->obj_x_vel = -this->obj_x_vel;
		}
	} else if (this->obj_x_vel == -1) {
		if (this->obj_x > 1) {
			this->obj_x--;
		} else {
			this->obj_x_vel = -this->obj_x_vel;
		}
	}

	if (this->obj_y_vel == 1) {
		if (this->obj_y < this->world_size-2) {
			this->obj_y++;
		} else {
			this->obj_y_vel = -this->obj_y_vel;
		}
	} else if (this->obj_y_vel == -1) {
		if (this->obj_y > 1) {
			this->obj_y--;
		} else {
			this->obj_y_vel = -this->obj_y_vel;
		}
	}

	this->action_queue.erase(this->action_queue.begin());
	this->action_queue.push_back(action);
}
