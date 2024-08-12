#include "run_helpers.h"

#include <cmath>
#include <iostream>

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
	uniform_int_distribution<int> num_actions_distribution(1, 100);
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

void train_model(WorldModel* world_model,
				 int starting_state_index,
				 vector<int>& new_state_indexes,
				 int ending_state_index,
				 int starting_action,
				 vector<int>& new_actions,
				 int ending_action) {
	vector<vector<double>> obs;
	vector<vector<int>> actions;
	uniform_int_distribution<int> num_actions_distribution(1, 100);
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
		   actions,
		   starting_state_index,
		   new_state_indexes,
		   ending_state_index,
		   starting_action,
		   new_actions,
		   ending_action);
}

double measure_model(WorldModel* world_model) {
	double sum_misguess = 0.0;
	int sum_num_actions = 0;
	uniform_int_distribution<int> num_actions_distribution(1, 100);
	uniform_int_distribution<int> action_distribution(0, 3);
	for (int r_index = 0; r_index < MEASURE_NUM_RUNS; r_index++) {
		WorldTruth world_truth;

		int num_actions = num_actions_distribution(generator);
		sum_num_actions += num_actions;

		vector<double> state_likelihood = world_model->starting_likelihood;

		for (int a_index = 0; a_index < num_actions; a_index++) {
			double obs = world_truth.vals[world_truth.curr_x][world_truth.curr_y];

			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				sum_misguess += state_likelihood[s_index] * abs(obs - world_model->states[s_index]->average_val);
			}

			int action = action_distribution(generator);
			world_truth.move(action);

			vector<vector<double>> curr_likelihood(world_model->states.size());
			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				curr_likelihood[s_index] = vector<double>(world_model->states.size());
			}

			double sum_likelihood = 0.0;
			for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
				double likelihood = state_likelihood[start_index]
					* (1.0 - abs(obs - world_model->states[start_index]->average_val));
				for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
					curr_likelihood[start_index][end_index] = likelihood
						* world_model->states[start_index]->transitions[action][end_index];

					sum_likelihood += curr_likelihood[start_index][end_index];
				}
			}

			vector<double> next_likelihood(world_model->states.size());
			for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
				double sum_end_likelihood = 0.0;
				for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
					sum_end_likelihood += curr_likelihood[start_index][end_index];
				}
				next_likelihood[end_index] = sum_end_likelihood / sum_likelihood;
			}

			state_likelihood = next_likelihood;
		}
	}

	return sum_misguess / sum_num_actions;
}
