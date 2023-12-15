#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action.h"
#include "constants.h"
#include "globals.h"
#include "world_model.h"
#include "world_state.h"

using namespace std;

default_random_engine generator;

WorldModel* world_model;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	world_model = new WorldModel();
	world_model->init();

	vector<Action*> actions;
	for (int a_index = 0; a_index < 3; a_index++) {
		actions.push_back(new Action());
		actions.back()->id = a_index;
		actions.back()->num_inputs = 0;
	}

	uniform_real_distribution<double> obs_distribution(-1.0, 1.0);
	geometric_distribution<int> geometric_distribution(0.3);
	uniform_int_distribution<int> action_distribution(0, 2);

	while (true) {
		double sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_NUM_SAMPLES; iter_index++) {
			vector<double> obs_sequence;
			vector<Action*> action_sequence;
			vector<vector<int>> action_state_sequence;
			vector<vector<double>> state_vals_sequence;
			int instance_length = 1 + geometric_distribution(generator);
			int num_0s = 0;
			for (int l_index = 0; l_index < instance_length; l_index++) {
				obs_sequence.push_back(obs_distribution(generator));

				Action* action = actions[action_distribution(generator)];
				action_sequence.push_back(action);
				if (action == actions[0]) {
					num_0s++;
				}

				action_state_sequence.push_back(vector<int>());

				state_vals_sequence.push_back(vector<double>());
			}

			double target_val;
			if (num_0s >= 3) {
				target_val = 1.0;
			} else {
				target_val = -1.0;
			}

			sum_misguess += world_model->measure_activate(
				obs_sequence,
				action_sequence,
				action_state_sequence,
				state_vals_sequence,
				target_val);
		}

		sum_misguess /= MEASURE_NUM_SAMPLES;
		cout << "misguess: " << sum_misguess << endl;

		while (true) {
			vector<double> obs_sequence;
			vector<Action*> action_sequence;
			vector<vector<int>> action_state_sequence;
			vector<vector<double>> state_vals_sequence;
			int instance_length = 1 + geometric_distribution(generator);
			int num_0s = 0;
			for (int l_index = 0; l_index < instance_length; l_index++) {
				obs_sequence.push_back(obs_distribution(generator));

				Action* action = actions[action_distribution(generator)];
				action_sequence.push_back(action);
				if (action == actions[0]) {
					num_0s++;
				}

				action_state_sequence.push_back(vector<int>());

				state_vals_sequence.push_back(vector<double>());
			}

			double target_val;
			if (num_0s >= 3) {
				target_val = 1.0;
			} else {
				target_val = -1.0;
			}

			bool is_success = world_model->activate(obs_sequence,
													action_sequence,
													action_state_sequence,
													state_vals_sequence,
													target_val);
			if (is_success) {
				break;
			}
		}
	}

	cout << "Done" << endl;
}
