#include "mapping_helpers.h"

#include "globals.h"
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
				uniform_int_distribution<int> y_change_distribution(-1, -2);
				assignment.y_impact.push_back(y_change_distribution(generator));
			}
			break;
		case ACTION_LEFT:
			{
				uniform_int_distribution<int> x_change_distribution(-1, -2);
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
				uniform_int_distribution<int> y_change_distribution(-1, -2);
				assignment.y_impact[modify_index] = y_change_distribution(generator);
			}
			break;
		case ACTION_LEFT:
			{
				uniform_int_distribution<int> x_change_distribution(-1, -2);
				assignment.x_impact[modify_index] = x_change_distribution(generator);
				uniform_int_distribution<int> y_change_distribution(-1, 1);
				assignment.y_impact[modify_index] = y_change_distribution(generator);
			}
			break;
		}
	}
}
