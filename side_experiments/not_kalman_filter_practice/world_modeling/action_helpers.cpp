#include "action_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

const double MATCH_ALL_WEIGHT = 1.0;

void apply_action(WorldTruth& world_truth,
				  int action,
				  int& new_obs) {
	int x_change;
	int y_change;
	switch (action) {
	case ACTION_LEFT:
		x_change = -1;
		y_change = 0;
		break;
	case ACTION_UP:
		x_change = 0;
		y_change = 1;
		break;
	case ACTION_RIGHT:
		x_change = 1;
		y_change = 0;
		break;
	case ACTION_DOWN:
		x_change = 0;
		y_change = -1;
		break;
	}

	uniform_int_distribution<int> is_noise_distribution(0, 1);
	uniform_int_distribution<int> noise_distribution(0, 4);
	if (is_noise_distribution(generator) == 0) {
		switch (noise_distribution(generator)) {
		case 0:
			x_change += 1;
			break;
		case 1:
			x_change -= 1;
			break;
		case 2:
			y_change += 1;
			break;
		case 3:
			y_change -= 1;
			break;
		}
	}

	world_truth.curr_x += x_change;
	if (world_truth.curr_x < 0) {
		world_truth.curr_x = 0;
	} else if (world_truth.curr_x > WORLD_X-1) {
		world_truth.curr_x = WORLD_X-1;
	}
	world_truth.curr_y += y_change;
	if (world_truth.curr_y < 0) {
		world_truth.curr_y = 0;
	} else if (world_truth.curr_y > WORLD_Y-1) {
		world_truth.curr_y = WORLD_Y-1;
	}

	new_obs = world_truth.vals[world_truth.curr_x][world_truth.curr_y];
}

void update_predicted(PredictedWorld& predicted_world,
					  int action,
					  int new_obs) {
	vector<vector<double>> initial_new_loc(WORLD_X);
	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		initial_new_loc[x_index] = vector<double>(WORLD_Y, 0.0);
	}

	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		for (int y_index = 0; y_index < WORLD_Y; y_index++) {
			double loc_weight = predicted_world.loc[x_index][y_index];
			if (loc_weight != 0.0) {
				switch (action) {
				case ACTION_LEFT:
					if (x_index-1 >= 0) {
						initial_new_loc[x_index-1][y_index] += 0.6 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index-1][y_index][new_obs] += 0.6 * loc_weight;
							predicted_world.val_counts[x_index-1][y_index][new_obs] += loc_weight;
						}
					}

					if (x_index-2 >= 0) {
						initial_new_loc[x_index-2][y_index] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index-2][y_index][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index-2][y_index][new_obs] += loc_weight;
						}
					}
					if (x_index-1 >= 0 && y_index+1 < WORLD_Y) {
						initial_new_loc[x_index-1][y_index+1] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index-1][y_index+1][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index-1][y_index+1][new_obs] += loc_weight;
						}
					}
					{
						initial_new_loc[x_index][y_index] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index][y_index][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index][y_index][new_obs] += loc_weight;
						}
					}
					if (x_index-1 >= 0 && y_index-1 >= 0) {
						initial_new_loc[x_index-1][y_index-1] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index-1][y_index-1][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index-1][y_index-1][new_obs] += loc_weight;
						}
					}

					break;
				case ACTION_UP:
					if (y_index+1 < WORLD_Y) {
						initial_new_loc[x_index][y_index+1] += 0.6 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index][y_index+1][new_obs] += 0.6 * loc_weight;
							predicted_world.val_counts[x_index][y_index+1][new_obs] += loc_weight;
						}
					}

					if (x_index-1 >= 0 && y_index+1 < WORLD_Y) {
						initial_new_loc[x_index-1][y_index+1] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index-1][y_index+1][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index-1][y_index+1][new_obs] += loc_weight;
						}
					}
					if (y_index+2 < WORLD_Y) {
						initial_new_loc[x_index][y_index+2] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index][y_index+2][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index][y_index+2][new_obs] += loc_weight;
						}
					}
					if (x_index+1 < WORLD_X && y_index+1 < WORLD_Y) {
						initial_new_loc[x_index+1][y_index+1] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index+1][y_index+1][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index+1][y_index+1][new_obs] += loc_weight;
						}
					}
					{
						initial_new_loc[x_index][y_index] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index][y_index][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index][y_index][new_obs] += loc_weight;
						}
					}

					break;
				case ACTION_RIGHT:
					if (x_index+1 < WORLD_X) {
						initial_new_loc[x_index+1][y_index] += 0.6 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index+1][y_index][new_obs] += 0.6 * loc_weight;
							predicted_world.val_counts[x_index+1][y_index][new_obs] += loc_weight;
						}
					}

					{
						initial_new_loc[x_index][y_index] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index][y_index][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index][y_index][new_obs] += loc_weight;
						}
					}
					if (x_index+1 < WORLD_X && y_index+1 < WORLD_Y) {
						initial_new_loc[x_index+1][y_index+1] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index+1][y_index+1][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index+1][y_index+1][new_obs] += loc_weight;
						}
					}
					if (x_index+2 < WORLD_X) {
						initial_new_loc[x_index+2][y_index] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index+2][y_index][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index+2][y_index][new_obs] += loc_weight;
						}
					}
					if (x_index+1 < WORLD_X && y_index-1 >= 0) {
						initial_new_loc[x_index+1][y_index-1] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index+1][y_index-1][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index+1][y_index-1][new_obs] += loc_weight;
						}
					}

					break;
				case ACTION_DOWN:
					if (y_index-1 >= 0) {
						initial_new_loc[x_index][y_index-1] += 0.6 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index][y_index-1][new_obs] += 0.6 * loc_weight;
							predicted_world.val_counts[x_index][y_index-1][new_obs] += loc_weight;
						}
					}

					if (x_index-1 >= 0 && y_index-1 >= 0) {
						initial_new_loc[x_index-1][y_index-1] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index-1][y_index-1][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index-1][y_index-1][new_obs] += loc_weight;
						}
					}
					{
						initial_new_loc[x_index][y_index] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index][y_index][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index][y_index][new_obs] += loc_weight;
						}
					}
					if (x_index+1 < WORLD_X && y_index-1 >= 0) {
						initial_new_loc[x_index+1][y_index-1] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index+1][y_index-1][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index+1][y_index-1][new_obs] += loc_weight;
						}
					}
					if (y_index-2 >= 0) {
						initial_new_loc[x_index][y_index-2] += 0.1 * loc_weight;

						if (new_obs != -10) {
							predicted_world.val_weights[x_index][y_index-2][new_obs] += 0.1 * loc_weight;
							predicted_world.val_counts[x_index][y_index-2][new_obs] += loc_weight;
						}
					}

					break;
				}
			}
		}
	}

	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		for (int y_index = 0; y_index < WORLD_Y; y_index++) {
			if (new_obs == -10) {
				if (x_index != 0
						&& x_index != WORLD_X-1
						&& y_index != 0
						&& y_index != WORLD_Y-1) {
					initial_new_loc[x_index][y_index] = 0.0;
				}
			} else {
				if (x_index == 0
						|| x_index == WORLD_X-1
						|| y_index == 0
						|| y_index == WORLD_Y-1) {
					initial_new_loc[x_index][y_index] = 0.0;
				} else {
					double sum_count = 0.0;
					for (int v_index = 0; v_index < 3; v_index++) {
						sum_count += predicted_world.val_counts[x_index][y_index][v_index];
					}
					double init_weight = MATCH_ALL_WEIGHT / sum_count;

					double match_sum = init_weight / 3 + predicted_world.val_weights[x_index][y_index][new_obs]
						/ predicted_world.val_counts[x_index][y_index][new_obs];
					double total_sum = init_weight;
					for (int v_index = 0; v_index < (int)predicted_world.val_weights[x_index][y_index].size(); v_index++) {
						total_sum += predicted_world.val_weights[x_index][y_index][v_index]
							/ predicted_world.val_counts[x_index][y_index][v_index];
					}

					initial_new_loc[x_index][y_index] *= (match_sum/total_sum);
				}
			}
		}
	}

	double sum_probability = 0.0;
	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		for (int y_index = 0; y_index < WORLD_Y; y_index++) {
			sum_probability += initial_new_loc[x_index][y_index];
		}
	}
	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		for (int y_index = 0; y_index < WORLD_Y; y_index++) {
			predicted_world.loc[x_index][y_index] = initial_new_loc[x_index][y_index] / sum_probability;
		}
	}
}
