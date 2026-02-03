#include "world.h"

#include <iostream>

#include "globals.h"

using namespace std;

World::World() {
	this->world = vector<vector<int>>(WORLD_WIDTH, vector<int>(WORLD_HEIGHT));

	uniform_int_distribution<int> input_distribution(-3, 3);
	for (int x_index = 0; x_index < WORLD_WIDTH; x_index++) {
		for (int y_index = 0; y_index < WORLD_HEIGHT; y_index++) {
			this->world[x_index][y_index] = input_distribution(generator);
		}
	}

	uniform_int_distribution<int> x_distribution(0, WORLD_WIDTH-1);
	this->curr_x = x_distribution(generator);
	uniform_int_distribution<int> y_distribution(0, WORLD_HEIGHT-1);
	this->curr_y = y_distribution(generator);
}

int World::get_observation() {
	return this->world[this->curr_x][this->curr_y];
}

void World::perform_action(int action) {
	switch (action) {
	case ACTION_UP:
		{
			uniform_int_distribution<int> x_change_distribution(-1, 1);
			this->curr_x += x_change_distribution(generator);
			uniform_int_distribution<int> y_change_distribution(1, 2);
			this->curr_y += y_change_distribution(generator);
		}
		break;
	case ACTION_RIGHT:
		{
			uniform_int_distribution<int> x_change_distribution(1, 2);
			this->curr_x += x_change_distribution(generator);
			uniform_int_distribution<int> y_change_distribution(-1, 1);
			this->curr_y += y_change_distribution(generator);
		}
		break;
	case ACTION_DOWN:
		{
			uniform_int_distribution<int> x_change_distribution(-1, 1);
			this->curr_x += x_change_distribution(generator);
			uniform_int_distribution<int> y_change_distribution(-2, -1);
			this->curr_y += y_change_distribution(generator);
		}
		break;
	case ACTION_LEFT:
		{
			uniform_int_distribution<int> x_change_distribution(-2, -1);
			this->curr_x += x_change_distribution(generator);
			uniform_int_distribution<int> y_change_distribution(-1, 1);
			this->curr_y += y_change_distribution(generator);
		}
		break;
	}

	if (this->curr_x < 0) {
		this->curr_x = 0;
	} else if (this->curr_x >= WORLD_WIDTH) {
		this->curr_x = WORLD_WIDTH-1;
	}
	if (this->curr_y < 0) {
		this->curr_y = 0;
	} else if (this->curr_y >= WORLD_HEIGHT) {
		this->curr_y = WORLD_HEIGHT-1;
	}
}

void World::print() {
	for (int x_index = 0; x_index < WORLD_WIDTH; x_index++) {
		for (int y_index = 0; y_index < WORLD_HEIGHT; y_index++) {
			cout << this->world[x_index][y_index] << " ";
		}
		cout << endl;
	}
	cout << "this->curr_x: " << this->curr_x << endl;
	cout << "this->curr_y: " << this->curr_y << endl;
}
