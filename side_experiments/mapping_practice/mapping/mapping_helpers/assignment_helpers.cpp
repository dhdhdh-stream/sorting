#include "mapping_helpers.h"

#include "globals.h"
#include "utilities.h"
#include "world.h"

using namespace std;

void init_assignment_helper(vector<int>& actions,
							Assignment& assignment) {
	uniform_int_distribution<int> x_distribution(0, WORLD_WIDTH-1);
	assignment.init_x = x_distribution(generator);
	uniform_int_distribution<int> y_distribution(0, WORLD_HEIGHT-1);
	assignment.init_y = y_distribution(generator);

	for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
		switch (actions[a_index]) {
		case ACTION_UP:
			{
				uniform_int_distribution<int> x_change_distribution(-1, 1);
				assignment.x_impact.push_back(x_change_distribution(generator));
				uniform_int_distribution<int> y_change_distribution(1, 2);
				assignment.y_impact.push_back(y_change_distribution(generator));
			}
			break;
		case ACTION_RIGHT:
			{
				uniform_int_distribution<int> x_change_distribution(1, 2);
				assignment.x_impact.push_back(x_change_distribution(generator));
				uniform_int_distribution<int> y_change_distribution(-1, 1);
				assignment.y_impact.push_back(y_change_distribution(generator));
			}
			break;
		case ACTION_DOWN:
			{
				uniform_int_distribution<int> x_change_distribution(-1, 1);
				assignment.x_impact.push_back(x_change_distribution(generator));
				uniform_int_distribution<int> y_change_distribution(-2, -1);
				assignment.y_impact.push_back(y_change_distribution(generator));
			}
			break;
		case ACTION_LEFT:
			{
				uniform_int_distribution<int> x_change_distribution(-2, -1);
				assignment.x_impact.push_back(x_change_distribution(generator));
				uniform_int_distribution<int> y_change_distribution(-1, 1);
				assignment.y_impact.push_back(y_change_distribution(generator));
			}
			break;
		}
	}
}

void modify_assignment_helper(vector<int>& actions,
							  Assignment& assignment) {
	uniform_int_distribution<int> modify_distribution(-1, actions.size()-1);
	int modify_index = modify_distribution(generator);
	if (modify_index == -1) {
		uniform_int_distribution<int> x_distribution(0, WORLD_WIDTH-1);
		assignment.init_x = x_distribution(generator);
		uniform_int_distribution<int> y_distribution(0, WORLD_HEIGHT-1);
		assignment.init_y = y_distribution(generator);
	} else {
		switch (actions[modify_index]) {
		case ACTION_UP:
			{
				uniform_int_distribution<int> x_change_distribution(-1, 1);
				assignment.x_impact[modify_index] = x_change_distribution(generator);
				uniform_int_distribution<int> y_change_distribution(1, 2);
				assignment.y_impact[modify_index] = y_change_distribution(generator);
			}
			break;
		case ACTION_RIGHT:
			{
				uniform_int_distribution<int> x_change_distribution(1, 2);
				assignment.x_impact[modify_index] = x_change_distribution(generator);
				uniform_int_distribution<int> y_change_distribution(-1, 1);
				assignment.y_impact[modify_index] = y_change_distribution(generator);
			}
			break;
		case ACTION_DOWN:
			{
				uniform_int_distribution<int> x_change_distribution(-1, 1);
				assignment.x_impact[modify_index] = x_change_distribution(generator);
				uniform_int_distribution<int> y_change_distribution(-2, -1);
				assignment.y_impact[modify_index] = y_change_distribution(generator);
			}
			break;
		case ACTION_LEFT:
			{
				uniform_int_distribution<int> x_change_distribution(-2, -1);
				assignment.x_impact[modify_index] = x_change_distribution(generator);
				uniform_int_distribution<int> y_change_distribution(-1, 1);
				assignment.y_impact[modify_index] = y_change_distribution(generator);
			}
			break;
		}
	}
}


void simple_assign(Mapping& mapping,
				   vector<vector<int>>& instance) {
	instance = vector<vector<int>>(WORLD_WIDTH, vector<int>(WORLD_HEIGHT));
	for (int x_index = 0; x_index < WORLD_WIDTH; x_index++) {
		for (int y_index = 0; y_index < WORLD_HEIGHT; y_index++) {
			int sum_count = 0;
			for (map<int,int>::iterator it = mapping.world[x_index][y_index].begin();
					it != mapping.world[x_index][y_index].end(); it++) {
				sum_count += it->second;
			}

			uniform_int_distribution<int> distribution(1, sum_count);
			int random_val = distribution(generator);
			for (map<int,int>::iterator it = mapping.world[x_index][y_index].begin();
					it != mapping.world[x_index][y_index].end(); it++) {
				random_val -= it->second;
				if (random_val <= 0) {
					instance[x_index][y_index] = it->first;
					break;
				}
			}
		}
	}
}

// TODO: not good enough
void simple_assign(vector<int>& obs,
				   vector<int>& actions,
				   vector<vector<int>>& mapping) {
	uniform_int_distribution<int> x_distribution(0, WORLD_WIDTH-1);
	int curr_x = x_distribution(generator);
	uniform_int_distribution<int> y_distribution(0, WORLD_HEIGHT-1);
	int curr_y = y_distribution(generator);

	vector<pair<pair<int,int>, int>> potential_assigns;

	potential_assigns.push_back({{curr_x, curr_y}, obs[0]});

	for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
		switch (actions[a_index]) {
		case ACTION_UP:
			{
				uniform_int_distribution<int> x_change_distribution(-1, 1);
				curr_x += x_change_distribution(generator);
				uniform_int_distribution<int> y_change_distribution(1, 2);
				curr_y += y_change_distribution(generator);
			}
			break;
		case ACTION_RIGHT:
			{
				uniform_int_distribution<int> x_change_distribution(1, 2);
				curr_x += x_change_distribution(generator);
				uniform_int_distribution<int> y_change_distribution(-1, 1);
				curr_y += y_change_distribution(generator);
			}
			break;
		case ACTION_DOWN:
			{
				uniform_int_distribution<int> x_change_distribution(-1, 1);
				curr_x += x_change_distribution(generator);
				uniform_int_distribution<int> y_change_distribution(-2, -1);
				curr_y += y_change_distribution(generator);
			}
			break;
		case ACTION_LEFT:
			{
				uniform_int_distribution<int> x_change_distribution(-2, -1);
				curr_x += x_change_distribution(generator);
				uniform_int_distribution<int> y_change_distribution(-1, 1);
				curr_y += y_change_distribution(generator);
			}
			break;
		}

		if (curr_x < 0) {
			curr_x = 0;
		} else if (curr_x >= WORLD_WIDTH) {
			curr_x = WORLD_WIDTH-1;
		}
		if (curr_y < 0) {
			curr_y = 0;
		} else if (curr_y >= WORLD_HEIGHT) {
			curr_y = WORLD_HEIGHT-1;
		}

		potential_assigns.push_back({{curr_x, curr_y}, obs[1 + a_index]});
	}

	mapping = vector<vector<int>>(WORLD_WIDTH, vector<int>(WORLD_HEIGHT, 0));
	vector<vector<bool>> assigned = vector<vector<bool>>(WORLD_WIDTH, vector<bool>(WORLD_HEIGHT, false));
	while (potential_assigns.size() > 0) {
		uniform_int_distribution<int> distribution(0, potential_assigns.size()-1);
		int index = distribution(generator);

		int x_loc = potential_assigns[index].first.first;
		int y_loc = potential_assigns[index].first.second;
		int obs = potential_assigns[index].second;
		if (!assigned[x_loc][y_loc]) {
			mapping[x_loc][y_loc] = obs;
			assigned[x_loc][y_loc] = true;
		}

		potential_assigns.erase(potential_assigns.begin() + index);
	}
}
