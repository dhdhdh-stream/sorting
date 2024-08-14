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

void examine_run(WorldModel* world_model) {
	WorldTruth world_truth;

	vector<vector<vector<double>>> state_likelihoods;
	vector<vector<double>> sum_likelihoods;
	vector<int> actions;

	sum_likelihoods.push_back(world_model->starting_likelihood);

	uniform_int_distribution<int> action_distribution(0, 3);
	for (int a_index = 0; a_index < 20; a_index++) {
		cout << a_index << endl;

		double obs = world_truth.vals[world_truth.curr_x][world_truth.curr_y];
		cout << "obs: " << obs << endl;

		vector<int> likeliest_path;

		double highest_likelihood = 0.0;
		int highest_index = -1;
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			double likelihood = sum_likelihoods.back()[s_index]
				* (1.0 - abs(obs - world_model->states[s_index]->average_val));
			if (likelihood > highest_likelihood) {
				highest_likelihood = likelihood;
				highest_index = s_index;
			}
		}
		likeliest_path.insert(likeliest_path.begin(), highest_index);

		for (int t_index = (int)state_likelihoods.size()-1; t_index >= 0; t_index--) {
			double highest_likelihood = 0.0;
			int highest_index = -1;
			for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
				double likelihood = state_likelihoods[t_index][start_index][likeliest_path[0]];
				if (likelihood > highest_likelihood) {
					highest_likelihood = likelihood;
					highest_index = start_index;
				}
			}
			likeliest_path.insert(likeliest_path.begin(), highest_index);
		}

		cout << "likeliest_path:";
		for (int p_index = 0; p_index < (int)likeliest_path.size(); p_index++) {
			cout << " " << likeliest_path[p_index];
		}
		cout << endl;

		int action = action_distribution(generator);
		actions.push_back(action);
		world_truth.move(action);
		switch (action) {
		case ACTION_UP:
			cout << "ACTION_UP" << endl;
			break;
		case ACTION_RIGHT:
			cout << "ACTION_RIGHT" << endl;
			break;
		case ACTION_DOWN:
			cout << "ACTION_DOWN" << endl;
			break;
		case ACTION_LEFT:
			cout << "ACTION_LEFT" << endl;
			break;
		}

		vector<vector<double>> curr_likelihood(world_model->states.size());
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			curr_likelihood[s_index] = vector<double>(world_model->states.size());
		}

		double sum_likelihood = 0.0;
		for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
			double likelihood = sum_likelihoods.back()[start_index]
				* (1.0 - abs(obs - world_model->states[start_index]->average_val));
			for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
				curr_likelihood[start_index][end_index] = likelihood
					* world_model->states[start_index]->transitions[action][end_index];

				sum_likelihood += curr_likelihood[start_index][end_index];
			}
		}
		state_likelihoods.push_back(curr_likelihood);

		vector<double> next_likelihood(world_model->states.size());
		for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
			double sum_end_likelihood = 0.0;
			for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
				sum_end_likelihood += curr_likelihood[start_index][end_index];
			}
			next_likelihood[end_index] = sum_end_likelihood / sum_likelihood;
		}

		cout << "likelihoods:" << endl;
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			cout << s_index << ": " << next_likelihood[s_index] << endl;
		}

		sum_likelihoods.push_back(next_likelihood);

		cout << endl;
	}
}
