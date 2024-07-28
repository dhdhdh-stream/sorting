#include "action_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "predicted_world.h"
#include "world_model.h"
#include "world_truth.h"

using namespace std;

const double PROBABILITY_CUT_OFF = 0.01;

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
	vector<WorldModel*> new_models;
	vector<double> new_weights;
	for (int w_index = 0; w_index < (int)predicted_world.possible_models.size(); w_index++) {
		vector<WorldModel*> next;
		vector<double> next_probabilities;
		predicted_world.possible_models[w_index]->next_probabilities(
			action,
			new_obs,
			next,
			next_probabilities);

		for (int n_index = 0; n_index < (int)next.size(); n_index++) {
			double adjusted_probability = predicted_world.possible_weights[w_index] * next_probabilities[n_index];

			int index = -1;
			for (int e_index = 0; e_index < (int)new_models.size(); e_index++) {
				if (new_models[e_index]->equals(next[n_index])) {
					index = e_index;
					break;
				}
			}

			if (index == -1) {
				new_models.push_back(next[n_index]);
				new_weights.push_back(adjusted_probability);
			} else {
				new_weights[index] += adjusted_probability;
				delete next[n_index];
			}
		}
	}

	double max_probability = 0.0;
	for (int n_index = 0; n_index < (int)new_models.size(); n_index++) {
		if (new_weights[n_index] > max_probability) {
			max_probability = new_weights[n_index];
		}
	}
	for (int n_index = (int)new_models.size()-1; n_index >= 0; n_index--) {
		new_weights[n_index] /= max_probability;
		if (new_weights[n_index] < PROBABILITY_CUT_OFF) {
			delete new_models[n_index];
			new_models.erase(new_models.begin() + n_index);
			new_weights.erase(new_weights.begin() + n_index);
		}
	}

	for (int p_index = 0; p_index < (int)predicted_world.possible_models.size(); p_index++) {
		delete predicted_world.possible_models[p_index];
	}

	predicted_world.possible_models = new_models;
	predicted_world.possible_weights = new_weights;
}
