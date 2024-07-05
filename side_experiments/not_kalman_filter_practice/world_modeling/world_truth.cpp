#include "world_truth.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

WorldTruth::WorldTruth() {
	this->vals = vector<vector<int>>(WORLD_X);
	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		this->vals[x_index] = vector<int>(WORLD_Y);
	}

	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		this->vals[x_index][0] = -10;
		this->vals[x_index][WORLD_Y-1] = -10;
	}

	for (int y_index = 1; y_index < WORLD_Y-1; y_index++) {
		this->vals[0][y_index] = -10;
		this->vals[WORLD_X-1][y_index] = -10;
	}

	uniform_int_distribution<int> val_distribution(0, 2);
	for (int x_index = 1; x_index < WORLD_X-1; x_index++) {
		for (int y_index = 1; y_index < WORLD_Y-1; y_index++) {
			this->vals[x_index][y_index] = val_distribution(generator);
		}
	}

	this->curr_x = 3;
	this->curr_y = 3;
}

void WorldTruth::print() {
	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		for (int y_index = 0; y_index < WORLD_Y; y_index++) {
			cout << this->vals[x_index][y_index] << " ";
		}
		cout << endl;
	}

	cout << "this->curr_x: " << this->curr_x << endl;
	cout << "this->curr_y: " << this->curr_y << endl;
}
