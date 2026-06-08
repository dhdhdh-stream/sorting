// probably best to train single, then split

#include <algorithm>
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

const int SAMPLES_PER_EPOCH = 5000;
const int MIN_SELECT = 50;
const int NUM_RANDOM = 50;

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
	uniform_int_distribution<int> sample_distribution(0, SAMPLES_PER_EPOCH-1);
	for (int epoch_index = 0; epoch_index < 100; epoch_index++) {
		vector<vector<double>> target_vals(SAMPLES_PER_EPOCH);
		for (int h_index = 0; h_index < SAMPLES_PER_EPOCH; h_index++) {
			vector<double> curr_target_val(NUM_DIMENSIONS);
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				curr_target_val[d_index] = target_distribution(generator);
			}
			target_vals[h_index] = curr_target_val;
		}

		vector<vector<double>> errors(SAMPLES_PER_EPOCH);
		for (int h_index = 0; h_index < SAMPLES_PER_EPOCH; h_index++) {
			vector<double> curr_errors(networks.size());
			vector<double> inputs;
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				networks[n_index]->activate(inputs);
				double sum_errors = 0.0;
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					sum_errors += (target_vals[h_index][d_index] - networks[n_index]->output->acti_vals[d_index])
						* (target_vals[h_index][d_index] - networks[n_index]->output->acti_vals[d_index]);
				}
				curr_errors[n_index] = sum_errors;
			}
			errors[h_index] = curr_errors;
		}

		vector<int> min_indexes(SAMPLES_PER_EPOCH);
		for (int h_index = 0; h_index < SAMPLES_PER_EPOCH; h_index++) {
			int min_index = 0;
			int min_error = errors[h_index][0];
			for (int n_index = 1; n_index < (int)networks.size(); n_index++) {
				if (errors[h_index][n_index] < min_error) {
					min_index = n_index;
					min_error = errors[h_index][n_index];
				}
			}
			min_indexes[h_index] = min_index;
		}

		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			for (int h_index = 0; h_index < SAMPLES_PER_EPOCH; h_index++) {
				if (min_indexes[h_index] == n_index) {
					vector<double> inputs;
					networks[n_index]->activate(inputs);
					vector<double> errors(NUM_DIMENSIONS);
					for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
						errors[d_index] = target_vals[h_index][d_index] - networks[n_index]->output->acti_vals[d_index];
					}
					networks[n_index]->backprop(errors);
				}
			}

			int num_selected = 0;
			for (int h_index = 0; h_index < SAMPLES_PER_EPOCH; h_index++) {
				if (min_indexes[h_index] == n_index) {
					num_selected++;
				}
			}
			if (num_selected < MIN_SELECT) {
				vector<pair<double,int>> sorted_errors;
				for (int h_index = 0; h_index < SAMPLES_PER_EPOCH; h_index++) {
					if (min_indexes[h_index] != n_index) {
						sorted_errors.push_back({errors[h_index][n_index], h_index});
					}
				}
				sort(sorted_errors.begin(), sorted_errors.end());

				int num_needed = MIN_SELECT - num_selected;
				for (int i_index = 0; i_index < num_needed; i_index++) {
					vector<double> inputs;
					networks[n_index]->activate(inputs);
					vector<double> errors(NUM_DIMENSIONS);
					for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
						errors[d_index] = target_vals[sorted_errors[i_index].second][d_index] - networks[n_index]->output->acti_vals[d_index];
					}
					networks[n_index]->backprop(errors);
				}
			}

			for (int r_index = 0; r_index < NUM_RANDOM; r_index++) {
				int sample_index = sample_distribution(generator);

				vector<double> inputs;
				networks[n_index]->activate(inputs);
				vector<double> errors(NUM_DIMENSIONS);
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					errors[d_index] = target_vals[sample_index][d_index] - networks[n_index]->output->acti_vals[d_index];
				}
				networks[n_index]->backprop(errors);
			}
		}

		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			vector<double> inputs;
			networks[n_index]->activate(inputs);
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				cout << n_index << " " << d_index << ": " << networks[n_index]->output->acti_vals[0] << endl;
			}
		}
		cout << endl;
	}

	cout << "Done" << endl;
}
