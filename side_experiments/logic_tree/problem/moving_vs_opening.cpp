#include "moving_vs_opening.h"

#include <algorithm>
#include <iostream>
#include <set>

#include "globals.h"

using namespace std;

const int WORLD_SIZE = 9;

void open_helper(int curr_x,
				 int curr_y,
				 vector<vector<double>>& world,
				 int& num_open) {
	if (curr_x >= 0
			&& curr_x < WORLD_SIZE
			&& curr_y >= 0
			&& curr_y < WORLD_SIZE) {
		if (world[curr_x][curr_y] == -10) {
			uniform_int_distribution<int> val_distribution(0, 9);
			world[curr_x][curr_y] = val_distribution(generator);
			num_open++;

			uniform_int_distribution<int> open_distribution(0, 5);

			if (open_distribution(generator) == 0) {
				open_helper(curr_x-1,
							curr_y-1,
							world,
							num_open);
			}

			if (open_distribution(generator) == 0) {
				open_helper(curr_x-1,
							curr_y,
							world,
							num_open);
			}

			if (open_distribution(generator) == 0) {
				open_helper(curr_x-1,
							curr_y+1,
							world,
							num_open);
			}

			if (open_distribution(generator) == 0) {
				open_helper(curr_x,
							curr_y+1,
							world,
							num_open);
			}

			if (open_distribution(generator) == 0) {
				open_helper(curr_x+1,
							curr_y+1,
							world,
							num_open);
			}

			if (open_distribution(generator) == 0) {
				open_helper(curr_x+1,
							curr_y,
							world,
							num_open);
			}

			if (open_distribution(generator) == 0) {
				open_helper(curr_x+1,
							curr_y-1,
							world,
							num_open);
			}

			if (open_distribution(generator) == 0) {
				open_helper(curr_x,
							curr_y-1,
							world,
							num_open);
			}
		}
	}
}

double get_observation_helper(int curr_x,
							  int curr_y,
							  vector<vector<double>>& world) {
	if (curr_x < 0
			|| curr_x > WORLD_SIZE-1
			|| curr_y < 0
			|| curr_y > WORLD_SIZE-1) {
		return -20.0;
	} else {
		return world[curr_x][curr_y];
	}
}

void MovingVsOpening::get_train_instance(vector<double>& obs,
										 double& target_val) {
	vector<vector<double>> starting_world(WORLD_SIZE, vector<double>(WORLD_SIZE, -10));
	int starting_num_open = 0;
	open_helper(4,
				4,
				starting_world,
				starting_num_open);

	vector<pair<int,int>> opened;
	for (int x_index = 0; x_index < WORLD_SIZE; x_index++) {
		for (int y_index = 0; y_index < WORLD_SIZE; y_index++) {
			if (starting_world[x_index][y_index] != -10) {
				opened.push_back({x_index, y_index});
			}
		}
	}

	set<pair<int,int>> possible_opens;
	for (int o_index = 0; o_index < (int)opened.size(); o_index++) {
		if (opened[o_index].first-1 >= 0
				&& opened[o_index].second-1 >= 0) {
			if (starting_world[opened[o_index].first-1][opened[o_index].second-1] == -10) {
				possible_opens.insert({opened[o_index].first-1, opened[o_index].second-1});
			}
		}

		if (opened[o_index].first-1 >= 0) {
			if (starting_world[opened[o_index].first-1][opened[o_index].second] == -10) {
				possible_opens.insert({opened[o_index].first-1, opened[o_index].second});
			}
		}

		if (opened[o_index].first-1 >= 0
				&& opened[o_index].second+1 < WORLD_SIZE) {
			if (starting_world[opened[o_index].first-1][opened[o_index].second+1] == -10) {
				possible_opens.insert({opened[o_index].first-1, opened[o_index].second+1});
			}
		}

		if (opened[o_index].second+1 < WORLD_SIZE) {
			if (starting_world[opened[o_index].first][opened[o_index].second+1] == -10) {
				possible_opens.insert({opened[o_index].first, opened[o_index].second+1});
			}
		}

		if (opened[o_index].first+1 < WORLD_SIZE
				&& opened[o_index].second+1 < WORLD_SIZE) {
			if (starting_world[opened[o_index].first+1][opened[o_index].second+1] == -10) {
				possible_opens.insert({opened[o_index].first+1, opened[o_index].second+1});
			}
		}

		if (opened[o_index].first+1 < WORLD_SIZE) {
			if (starting_world[opened[o_index].first+1][opened[o_index].second] == -10) {
				possible_opens.insert({opened[o_index].first+1, opened[o_index].second});
			}
		}

		if (opened[o_index].first+1 < WORLD_SIZE
				&& opened[o_index].second-1 >= 0) {
			if (starting_world[opened[o_index].first+1][opened[o_index].second-1] == -10) {
				possible_opens.insert({opened[o_index].first+1, opened[o_index].second-1});
			}
		}

		if (opened[o_index].second-1 >= 0) {
			if (starting_world[opened[o_index].first][opened[o_index].second-1] == -10) {
				possible_opens.insert({opened[o_index].first, opened[o_index].second-1});
			}
		}
	}

	vector<vector<double>> ending_world = starting_world;
	int ending_num_open = starting_num_open;

	int ending_x;
	int ending_y;
	if (possible_opens.size() > 0) {
		uniform_int_distribution<int> open_distribution(0, possible_opens.size()-1);
		pair<int,int> open = *next(possible_opens.begin(), open_distribution(generator));

		ending_x = open.first;
		ending_y = open.second;

		uniform_int_distribution<int> new_open_distribution(0, 1);
		if (new_open_distribution(generator) == 0) {
			open_helper(open.first,
						open.second,
						ending_world,
						ending_num_open);
		}
	} else {
		ending_x = 4;
		ending_y = 4;
	}

	for (int x_index = -2; x_index <= 2; x_index++) {
		for (int y_index = -2; y_index <= 2; y_index++) {
			obs.push_back(get_observation_helper(
				4 + x_index,
				4 + y_index,
				starting_world
			));
		}
	}

	for (int x_index = -2; x_index <= 2; x_index++) {
		for (int y_index = -2; y_index <= 2; y_index++) {
			obs.push_back(get_observation_helper(
				ending_x + x_index,
				ending_y + y_index,
				ending_world
			));
		}
	}

	target_val = ending_num_open;
}

void MovingVsOpening::get_test_instance(vector<double>& obs,
										double& target_val) {
	get_train_instance(obs,
					   target_val);
}
