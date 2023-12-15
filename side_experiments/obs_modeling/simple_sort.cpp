/**
 * 0: focus left
 * 1: focus right
 * 2: swap
 */

#include <algorithm>
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

	uniform_int_distribution<int> val_distribution(0, 2);
	geometric_distribution<int> geometric_distribution(0.3);
	uniform_int_distribution<int> action_distribution(0, 2);

	while (true) {
		double sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_NUM_SAMPLES; iter_index++) {
			vector<int> underlying(2);
			for (int i = 0; i < 2; i++) {
				underlying[i] = val_distribution(generator);
			}
			bool curr_focus_is_left = true;

			vector<int> target = underlying;
			sort(target.begin(), target.end());

			vector<double> obs_sequence;
			vector<Action*> action_sequence;
			vector<vector<int>> action_state_sequence;
			vector<vector<double>> state_vals_sequence;
			int instance_length = 1 + geometric_distribution(generator);
			for (int l_index = 0; l_index < instance_length; l_index++) {
				if (curr_focus_is_left) {
					obs_sequence.push_back(underlying[0]);
				} else {
					obs_sequence.push_back(underlying[1]);
				}

				Action* action = actions[action_distribution(generator)];
				action_sequence.push_back(action);

				if (action->id == 0) {
					curr_focus_is_left = true;
				} else if (action->id == 1) {
					curr_focus_is_left = false;
				} else {
					underlying[0] -= 1;
					underlying[1] += 1;
				}

				action_state_sequence.push_back(vector<int>());

				state_vals_sequence.push_back(vector<double>());
			}

			double target_val;
			if (underlying == target) {
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
			vector<int> underlying(2);
			for (int i = 0; i < 2; i++) {
				underlying[i] = val_distribution(generator);
			}
			bool curr_focus_is_left = true;

			vector<int> target = underlying;
			sort(target.begin(), target.end());

			vector<double> obs_sequence;
			vector<Action*> action_sequence;
			vector<vector<int>> action_state_sequence;
			vector<vector<double>> state_vals_sequence;
			int instance_length = 1 + geometric_distribution(generator);
			for (int l_index = 0; l_index < instance_length; l_index++) {
				if (curr_focus_is_left) {
					obs_sequence.push_back(underlying[0]);
				} else {
					obs_sequence.push_back(underlying[1]);
				}

				Action* action = actions[action_distribution(generator)];
				action_sequence.push_back(action);

				if (action->id == 0) {
					curr_focus_is_left = true;
				} else if (action->id == 1) {
					curr_focus_is_left = false;
				} else {
					underlying[0] -= 1;
					underlying[1] += 1;
				}

				action_state_sequence.push_back(vector<int>());

				state_vals_sequence.push_back(vector<double>());
			}

			double target_val;
			if (underlying == target) {
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
