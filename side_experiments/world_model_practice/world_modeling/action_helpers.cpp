#include "action_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

void apply_action(WorldTruth& world_truth,
				  int action,
				  int& new_obs) {
	switch (action) {
	case ACTION_LEFT:
		if (world_truth.vals[world_truth.curr_x-1][world_truth.curr_y] != 0) {
			world_truth.curr_x--;
		}
		break;
	case ACTION_UP:
		if (world_truth.vals[world_truth.curr_x][world_truth.curr_y+1] != 0) {
			world_truth.curr_y++;
		}
		break;
	case ACTION_RIGHT:
		if (world_truth.vals[world_truth.curr_x+1][world_truth.curr_y] != 0) {
			world_truth.curr_x++;
		}
		break;
	case ACTION_DOWN:
		if (world_truth.vals[world_truth.curr_x][world_truth.curr_y-1] != 0) {
			world_truth.curr_y--;
		}
		break;
	}

	new_obs = world_truth.vals[world_truth.curr_x][world_truth.curr_y];
}

void update_predicted(PredictedWorld& predicted_world,
					  int action,
					  int new_obs) {
	for (int w_index = 0; w_index < (int)predicted_world.possible_models.size(); w_index++) {
		
	}
}
