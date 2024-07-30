#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "update_helpers.h"
#include "world_model.h"
#include "world_state.h"
#include "world_truth.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_STATES = 5;
// const int NUM_STATES = 16;

const int NUM_RUNS = 100;
const int NUM_MEASURE_RUNS = 10;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	uniform_real_distribution<double> distribution(0.0, 1.0);

	WorldModel* world_model = new WorldModel();

	for (int s_index = 0; s_index < NUM_STATES; s_index++) {
		WorldState* world_state = new WorldState();

		world_state->average_val = distribution(generator);

		world_state->transitions = vector<vector<double>>(4);
		for (int a_index = 0; a_index < 4; a_index++) {
			vector<double> likelihoods(NUM_STATES);

			double sum_likelihood = 0.0;
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				likelihoods[s_index] = distribution(generator);

				sum_likelihood += likelihoods[s_index];
			}

			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				likelihoods[s_index] /= sum_likelihood;
			}

			world_state->transitions[a_index] = likelihoods;
		}

		world_model->states.push_back(world_state);
	}

	{
		world_model->starting_likelihood = vector<double>(NUM_STATES);

		double sum_likelihood = 0.0;
		for (int s_index = 0; s_index < NUM_STATES; s_index++) {
			world_model->starting_likelihood[s_index] = distribution(generator);

			sum_likelihood += world_model->starting_likelihood[s_index];
		}

		for (int s_index = 0; s_index < NUM_STATES; s_index++) {
			world_model->starting_likelihood[s_index] /= sum_likelihood;
		}
	}

	vector<vector<double>> obs;
	vector<vector<int>> actions;
	uniform_int_distribution<int> num_actions_distribution(1, 100);
	uniform_int_distribution<int> action_distribution(0, 3);
	for (int r_index = 0; r_index < NUM_RUNS; r_index++) {
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

	double sum_misguess = 0.0;
	for (int r_index = 0; r_index < NUM_MEASURE_RUNS; r_index++) {
		WorldTruth world_truth;

		int num_actions = num_actions_distribution(generator);

		vector<double> state_likelihood = world_model->starting_likelihood;

		for (int a_index = 0; a_index < num_actions; a_index++) {
			cout << "curr_x: " << world_truth.curr_x << endl;
			cout << "curr_y: " << world_truth.curr_y << endl;

			double obs = world_truth.vals[world_truth.curr_x][world_truth.curr_y];

			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				sum_misguess += state_likelihood[s_index] * abs(obs - world_model->states[s_index]->average_val);

				cout << state_likelihood[s_index] << ": " << world_model->states[s_index]->average_val << endl;
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

	cout << "sum_misguess: " << sum_misguess << endl;

	delete world_model;

	cout << "Done" << endl;
}
