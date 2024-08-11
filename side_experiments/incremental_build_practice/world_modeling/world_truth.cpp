#include "world_truth.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

const int WORLD_X = 4;
const int WORLD_Y = 4;
// const int WORLD_X = 2;
// const int WORLD_Y = 2;

WorldTruth::WorldTruth() {
	this->vals = vector<vector<int>>(WORLD_X);
	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		this->vals[x_index] = vector<int>(WORLD_Y);
	}

	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		for (int y_index = 0; y_index < WORLD_Y; y_index++) {
			if (x_index >= 1
					&& x_index <= 2
					&& y_index >= 1
					&& y_index <= 2) {
				this->vals[x_index][y_index] = 1.0;
			} else {
				this->vals[x_index][y_index] = 0.0;
			}
		}
	}

	// this->curr_x = 1;
	// this->curr_y = 1;

	uniform_int_distribution<int> coord_distribution(0, 3);
	this->curr_x = coord_distribution(generator);
	this->curr_y = coord_distribution(generator);

	// for (int x_index = 0; x_index < WORLD_X; x_index++) {
	// 	for (int y_index = 0; y_index < WORLD_Y; y_index++) {
	// 		if ((x_index == 1 && y_index == 0)
	// 				|| (x_index == 0 && y_index == 1)) {
	// 			this->vals[x_index][y_index] = 1.0;
	// 		} else {
	// 			this->vals[x_index][y_index] = 0.0;
	// 		}
	// 	}
	// }

	// this->curr_x = 0;
	// this->curr_y = 0;
}

void WorldTruth::move(int action) {
	switch (action) {
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
}
