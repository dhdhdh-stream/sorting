#include "world_truth.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

const int DIRECTION_UP = 0;
const int DIRECTION_LEFT = 1;
const int DIRECTION_DOWN = 2;
const int DIRECTION_RIGHT = 3;

WorldTruth::WorldTruth() {
	// do nothing
}

void WorldTruth::init() {
	uniform_int_distribution<int> world_size_distribution(5, 7);
	this->world_size = world_size_distribution(generator);

	uniform_int_distribution<int> starting_distribution(0, this->world_size-1);
	this->curr_x = starting_distribution(generator);
	this->curr_y = starting_distribution(generator);

	uniform_int_distribution<int> direction_distribution(0, 3);
	this->curr_direction = direction_distribution(generator);

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
	} else if (this->curr_x == 1
			&& this->curr_y == 1) {
		return -1.0;
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
	case ACTION_TURN_LEFT:
		if (stay_distribution(generator) != 0) {
			this->curr_direction = (this->curr_direction - 1 + 4) % 4;
		}
		break;
	case ACTION_TURN_RIGHT:
		if (stay_distribution(generator) != 0) {
			this->curr_direction = (this->curr_direction + 1) % 4;
		}
		break;
	case ACTION_STEP:
		switch (this->curr_direction) {
		case DIRECTION_UP:
			if (this->curr_y < this->world_size-1) {
				if (stay_distribution(generator) != 0) {
					this->curr_y++;
				}
			}
			break;
		case DIRECTION_LEFT:
			if (this->curr_x > 0) {
				if (stay_distribution(generator) != 0) {
					this->curr_x--;
				}
			}
			break;
		case DIRECTION_DOWN:
			if (this->curr_y > 0) {
				if (stay_distribution(generator) != 0) {
					this->curr_y--;
				}
			}
			break;
		case DIRECTION_RIGHT:
			if (this->curr_x < this->world_size-1) {
				if (stay_distribution(generator) != 0) {
					this->curr_x++;
				}
			}
			break;
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

void WorldTruth::save(ofstream& output_file) {
	output_file << this->world_size << endl;
	output_file << this->curr_x << endl;
	output_file << this->curr_y << endl;
	output_file << this->curr_direction << endl;

	output_file << this->obj_x << endl;
	output_file << this->obj_y << endl;
	output_file << this->obj_x_vel << endl;
	output_file << this->obj_y_vel << endl;

	output_file << this->action_queue.size() << endl;
	for (int a_index = 0; a_index < (int)this->action_queue.size(); a_index++) {
		output_file << this->action_queue[a_index] << endl;
	}
}

void WorldTruth::load(ifstream& input_file) {
	string world_size_line;
	getline(input_file, world_size_line);
	this->world_size = stoi(world_size_line);

	string curr_x_line;
	getline(input_file, curr_x_line);
	this->curr_x = stoi(curr_x_line);

	string curr_y_line;
	getline(input_file, curr_y_line);
	this->curr_y = stoi(curr_y_line);

	string curr_direction_line;
	getline(input_file, curr_direction_line);
	this->curr_direction = stoi(curr_direction_line);

	string obj_x_line;
	getline(input_file, obj_x_line);
	this->obj_x = stoi(obj_x_line);

	string obj_y_line;
	getline(input_file, obj_y_line);
	this->obj_y = stoi(obj_y_line);

	string obj_x_vel_line;
	getline(input_file, obj_x_vel_line);
	this->obj_x_vel = stoi(obj_x_vel_line);

	string obj_y_vel_line;
	getline(input_file, obj_y_vel_line);
	this->obj_y_vel = stoi(obj_y_vel_line);

	string action_queue_length_line;
	getline(input_file, action_queue_length_line);
	int action_queue_length = stoi(action_queue_length_line);
	for (int a_index = 0; a_index < action_queue_length; a_index++) {
		string action_line;
		getline(input_file, action_line);
		this->action_queue.push_back(stoi(action_line));
	}
}
