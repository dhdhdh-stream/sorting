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
	/**
	 * - number has to be kept relatively low
	 *   - can't be spending too much time deep into unknown
	 *     - makes confident progress difficult
	 * 
	 * TODO: control explore
	 */
	uniform_int_distribution<int> num_actions_distribution(1, 20);
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
	uniform_int_distribution<int> num_actions_distribution(1, 20);
	uniform_int_distribution<int> action_distribution(0, 3);
	for (int r_index = 0; r_index < MEASURE_NUM_RUNS; r_index++) {
		WorldTruth world_truth;

		int num_actions = num_actions_distribution(generator);
		sum_num_actions += num_actions;

		vector<double> state_likelihood = world_model->starting_likelihood;
		double unknown_state_likelihood = 0.0;

		for (int a_index = 0; a_index < num_actions; a_index++) {
			double obs = world_truth.vals[world_truth.curr_x][world_truth.curr_y];

			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				sum_misguess += state_likelihood[s_index] * abs(obs - world_model->states[s_index]->average_val);
			}

			sum_misguess += unknown_state_likelihood * world_model->curr_unknown_misguess;

			int action = action_distribution(generator);
			world_truth.move(action);

			vector<vector<double>> curr_likelihood(world_model->states.size());
			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				curr_likelihood[s_index] = vector<double>(world_model->states.size());
			}
			vector<double> curr_to_unknown_likelihood(world_model->states.size());
			vector<double> curr_from_unknown_likelihood(world_model->states.size());
			double curr_unknown_to_unknown_likelihood;

			double sum_likelihood = 0.0;

			for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
				double likelihood = state_likelihood[start_index]
					* (1.0 - abs(obs - world_model->states[start_index]->average_val));
				for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
					curr_likelihood[start_index][end_index] = likelihood
						* world_model->states[start_index]->transitions[action][end_index];

					sum_likelihood += curr_likelihood[start_index][end_index];
				}

				{
					curr_to_unknown_likelihood[start_index] = likelihood
						* world_model->states[start_index]->unknown_transitions[action];

					sum_likelihood += curr_to_unknown_likelihood[start_index];
				}
			}

			{
				double unknown_likelihood = unknown_state_likelihood * (1.0 - world_model->curr_unknown_misguess);
				for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
					curr_from_unknown_likelihood[end_index] = unknown_likelihood
						* (1.0 - UNKNOWN_TO_UNKNOWN_TRANSITION) / (int)world_model->states.size();

					sum_likelihood += curr_from_unknown_likelihood[end_index];
				}

				{
					curr_unknown_to_unknown_likelihood = unknown_likelihood * UNKNOWN_TO_UNKNOWN_TRANSITION;

					sum_likelihood += curr_unknown_to_unknown_likelihood;
				}
			}

			vector<double> next_likelihood(world_model->states.size());
			double next_unknown_likelihood;

			for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
				double sum_end_likelihood = 0.0;

				for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
					sum_end_likelihood += curr_likelihood[start_index][end_index];
				}

				sum_end_likelihood += curr_from_unknown_likelihood[end_index];

				next_likelihood[end_index] = sum_end_likelihood / sum_likelihood;
			}

			{
				double sum_end_unknown_likelihood = 0.0;

				for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
					sum_end_unknown_likelihood += curr_to_unknown_likelihood[start_index];
				}

				sum_end_unknown_likelihood += curr_unknown_to_unknown_likelihood;

				next_unknown_likelihood = sum_end_unknown_likelihood / sum_likelihood;
			}

			state_likelihood = next_likelihood;
			unknown_state_likelihood = next_unknown_likelihood;
		}
	}

	double misguess = sum_misguess / sum_num_actions;

	world_model->curr_unknown_misguess = (misguess + 0.5) / 2.0;

	return misguess;
}
