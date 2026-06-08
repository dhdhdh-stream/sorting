// TODO: to train variance, start from assigning randomly 50% of the time, and going down?
// - or maybe just always 1% random assign

// - variance only use for gen
//   - not needed for prediction accuracy?
//     - for prediction accuracy, use sample variance maybe

// - networks can't be all selected
//   - becomes useless
//     - should latch on to a direction at least
// - maybe have 1% chance 2nd best network takes it

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_DIMENSIONS = 2;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<Network*> networks;
	networks.push_back(new Network(0, 2));
	networks.push_back(new Network(0, 2));
	networks.push_back(new Network(0, 2));
	networks.push_back(new Network(0, 2));

	uniform_int_distribution<int> target_distribution(0, 1);
	// uniform_int_distribution<int> target_distribution(0, 9);
	uniform_int_distribution<int> random_distribution(0, 99);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		vector<vector<double>> predicted(networks.size());
		vector<double> inputs;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			networks[n_index]->activate(inputs);
			vector<double> curr_predicted(NUM_DIMENSIONS);
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				curr_predicted[d_index] = networks[n_index]->output->acti_vals[d_index];
			}
			predicted[n_index] = curr_predicted;
		}

		vector<double> target_val(NUM_DIMENSIONS);
		for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
			target_val[d_index] = target_distribution(generator);
		}

		int min_index;
		int selection_type_rand = random_distribution(generator);
		if (selection_type_rand == 0) {
			uniform_int_distribution<int> distribution(0, networks.size()-1);
			min_index = distribution(generator);
		} else if (selection_type_rand == 1) {
			int best_min_index;
			double best_min_error = numeric_limits<double>::max();
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				double curr_error = 0.0;
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					curr_error += (target_val[d_index] - predicted[n_index][d_index])
						* (target_val[d_index] - predicted[n_index][d_index]);
				}
				if (curr_error < best_min_error) {
					best_min_error = curr_error;
					best_min_index = n_index;
				}
			}

			double min_error = numeric_limits<double>::max();
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				if (n_index != best_min_index) {
					double curr_error = 0.0;
					for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
						curr_error += (target_val[d_index] - predicted[n_index][d_index])
							* (target_val[d_index] - predicted[n_index][d_index]);
					}
					if (curr_error < min_error) {
						min_error = curr_error;
						min_index = n_index;
					}
				}
			}
		} else {
			double min_error = numeric_limits<double>::max();
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				double curr_error = 0.0;
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					curr_error += (target_val[d_index] - predicted[n_index][d_index])
						* (target_val[d_index] - predicted[n_index][d_index]);
				}
				if (curr_error < min_error) {
					min_error = curr_error;
					min_index = n_index;
				}
			}
		}

		vector<double> errors(NUM_DIMENSIONS);
		for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
			errors[d_index] = target_val[d_index] - networks[min_index]->output->acti_vals[d_index];
		}
		networks[min_index]->backprop(errors);

		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
			cout << "min_index: " << min_index << endl;
			cout << "target_val[0]: " << target_val[0] << endl;
			cout << "target_val[1]: " << target_val[1] << endl;
			// temp
			cout << "errors[0]: " << errors[0] << endl;
			cout << "errors[1]: " << errors[1] << endl;
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					cout << n_index << " " << d_index << ": " << predicted[n_index][d_index] << endl;
				}
			}
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
