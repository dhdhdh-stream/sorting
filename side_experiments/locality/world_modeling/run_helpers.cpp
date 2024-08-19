#include "run_helpers.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "update_helpers.h"
#include "world_model.h"
#include "world_state.h"
#include "world_truth.h"

using namespace std;

const int TRAIN_NUM_RUNS = 1000;
const int MEASURE_NUM_RUNS = 100;

void train_model(WorldModel* world_model) {
	vector<vector<double>> obs;
	vector<vector<int>> actions;
	uniform_int_distribution<int> num_actions_distribution(1, 40);
	uniform_int_distribution<int> action_distribution(0, 3);
	for (int r_index = 0; r_index < TRAIN_NUM_RUNS; r_index++) {
		vector<double> curr_obs;
		vector<int> curr_actions;

		WorldTruth world_truth;

		int num_actions = num_actions_distribution(generator);
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_obs.push_back(world_truth.vals[world_truth.curr_x][world_truth.curr_y]);

			int action = action_distribution(generator);
			curr_actions.push_back(action);
			world_truth.move(action);
		}

		obs.push_back(curr_obs);
		actions.push_back(curr_actions);
	}

	update(world_model,
		   obs,
		   actions);
}

double measure_model(WorldModel* world_model) {
	double sum_misguess = 0.0;
	int sum_num_actions = 0;
	uniform_int_distribution<int> num_actions_distribution(1, 40);
	uniform_int_distribution<int> action_distribution(0, 3);
	for (int r_index = 0; r_index < MEASURE_NUM_RUNS; r_index++) {
		WorldTruth world_truth;

		int num_actions = num_actions_distribution(generator);
		sum_num_actions += num_actions;

		vector<double> curr_likelihoods = world_model->starting_likelihood;
		double curr_unknown_likelihood = 0.0;

		for (int a_index = 0; a_index < num_actions; a_index++) {
			double obs = world_truth.vals[world_truth.curr_x][world_truth.curr_y];

			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				sum_misguess += curr_likelihoods[s_index] * abs(obs - world_model->states[s_index]->average_val);
			}
			sum_misguess += curr_unknown_likelihood * 0.5;

			int action = action_distribution(generator);
			world_truth.move(action);

			vector<double> next_likelihoods(world_model->states.size(), 0.0);
			double next_unknown_likelihood = 0.0;

			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				world_model->states[s_index]->forward(obs,
													  curr_likelihoods,
													  action,
													  next_likelihoods,
													  next_unknown_likelihood);
			}
			{
				double unknown_state_likelihood = curr_unknown_likelihood;
				next_unknown_likelihood += unknown_state_likelihood;
			}

			double sum_likelihood = 0.0;
			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				sum_likelihood += next_likelihoods[s_index];
			}
			sum_likelihood += next_unknown_likelihood;

			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				next_likelihoods[s_index] /= sum_likelihood;
			}
			next_unknown_likelihood /= sum_likelihood;

			curr_likelihoods = next_likelihoods;
			curr_unknown_likelihood = next_unknown_likelihood;
		}
	}

	return sum_misguess / sum_num_actions;
}
