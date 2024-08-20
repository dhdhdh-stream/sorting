#include "world_truth.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

const int WORLD_X = 2;
const int WORLD_Y = 2;

WorldTruth::WorldTruth() {
	this->vals = vector<vector<int>>(WORLD_X);
	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		this->vals[x_index] = vector<int>(WORLD_Y);
	}

	this->vals[0][0] = 1.0;
	this->vals[0][1] = 0.0;
	this->vals[1][0] = 0.0;
	this->vals[1][1] = 1.0;

	uniform_int_distribution<int> distribution(0, 1);
	this->curr_x = distribution(generator);
	this->curr_y = distribution(generator);

	this->action_queue.push_back(ACTION_NOOP);
	this->action_queue.push_back(ACTION_NOOP);
	this->action_queue.push_back(ACTION_NOOP);
}

void WorldTruth::move(int action) {
	switch (this->action_queue[0]) {
	case ACTION_LEFT:
		if (this->curr_x > 0) {
			this->curr_x--;
		}
		break;
	case ACTION_UP:
		if (this->curr_y < WORLD_Y-1) {
			this->curr_y++;
		}
		break;
	case ACTION_RIGHT:
		if (this->curr_x < WORLD_X-1) {
			this->curr_x++;
		}
		break;
	case ACTION_DOWN:
		if (this->curr_y > 0) {
			this->curr_y--;
		}
		break;
	}

	this->action_queue.erase(this->action_queue.begin());
	this->action_queue.push_back(action);
}
