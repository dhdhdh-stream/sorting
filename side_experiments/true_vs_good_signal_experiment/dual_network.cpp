// - true network goes in both directions
//   - if it's confident something is good or is bad

// - not that good
//   - doesn't reject aggressively enough low
//   - doesn't accept aggressively enough high
// - or limited in power
//   - still doesn't save from overwhelming noise

// - maybe try 1-dimensional clustering
// - maybe only makes sense when there are clear outliers at the top and bottom
//   - and signal handles the middle
//   - when this is not the case, then signals don't make sense anyways
// - can't really rely too much on a validation data set
//   - so yeah, seems reasonable

// - or actually, seems decent
//   - noise was just messing up the measurement

// - maybe on validation data, measure if significant change to signal
//   - if so, then dual setup with 1-dimensional clustering
//   - otherwise, just use true

// - also compare between just true, just signal, and dual on validation set

// TODO: practice k means on some handmade data

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

// const double LARGE_IMPACT = 10.0;
// const double LARGE_IMPACT = 100.0;
const double LARGE_IMPACT = 1000.0;
// const double SMALL_IMPACT = 1.0;
const double SMALL_IMPACT = 10.0;

const int NUM_INPUTS = 10;

const int NUM_TRAIN_SAMPLES = 1000;
const int NUM_VALIDATION_SAMPLES = 200;
const int NUM_TEST_SAMPLES = 200;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	int num_true_better = 0;
	int num_dual_better = 0;

	int num_true_underlying_better = 0;
	int num_dual_underlying_better = 0;

	uniform_int_distribution<int> noise_distribution(-100, 100);

	for (int epoch_index = 0; epoch_index < 1000; epoch_index++) {
		vector<vector<double>> train_obs;
		vector<double> train_vals;
		for (int i_index = 0; i_index < NUM_TRAIN_SAMPLES; i_index++) {
			double large_obs = rand()%3 - 1;
			double small_obs = rand()%3 - 1;
			double noise = noise_distribution(generator);
			double result = large_obs * LARGE_IMPACT + small_obs * SMALL_IMPACT + noise;

			vector<double> input;
			input.push_back(large_obs);
			input.push_back(small_obs);
			for (int ii_index = 0; ii_index < NUM_INPUTS-2; ii_index++) {
				input.push_back(rand()%3 - 1);
			}

			train_obs.push_back(input);
			train_vals.push_back(result);
		}

		Network* network = new Network(NUM_INPUTS,
									   NETWORK_SIZE_SMALL);
		uniform_int_distribution<int> train_distribution(0, NUM_TRAIN_SAMPLES-1);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			network->activate(train_obs[rand_index]);

			double error = train_vals[rand_index] - network->output->acti_vals[0];

			network->backprop(error);
		}

		// temp
		for (int i_index = 0; i_index < 10; i_index++) {
			double large_obs = rand()%3 - 1;
			double small_obs = rand()%3 - 1;
			double noise = noise_distribution(generator);
			double result = large_obs * LARGE_IMPACT + small_obs * SMALL_IMPACT + noise;

			vector<double> input;
			input.push_back(large_obs);
			input.push_back(small_obs);
			for (int ii_index = 0; ii_index < NUM_INPUTS-2; ii_index++) {
				input.push_back(rand()%3 - 1);
			}

			network->activate(input);

			cout << "large_obs: " << large_obs << endl;
			cout << "small_obs: " << small_obs << endl;
			cout << "noise: " << noise << endl;
			cout << "result: " << result << endl;
			cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
			cout << endl;
		}

		vector<vector<double>> validation_obs;
		vector<double> validation_vals;
		for (int i_index = 0; i_index < NUM_VALIDATION_SAMPLES; i_index++) {
			double large_obs = rand()%3 - 1;
			double small_obs = rand()%3 - 1;
			double noise = noise_distribution(generator);
			double result = large_obs * LARGE_IMPACT + small_obs * SMALL_IMPACT + noise;

			vector<double> input;
			input.push_back(large_obs);
			input.push_back(small_obs);
			for (int ii_index = 0; ii_index < NUM_INPUTS-2; ii_index++) {
				input.push_back(rand()%3 - 1);
			}

			validation_obs.push_back(input);
			validation_vals.push_back(result);
		}

		vector<pair<double,pair<double,double>>> validation_network_vals(validation_vals.size());
		for (int i_index = 0; i_index < (int)validation_vals.size(); i_index++) {
			network->activate(validation_obs[i_index]);
			validation_network_vals[i_index] = {network->output->acti_vals[0], {validation_vals[i_index], validation_obs[i_index][1]}};
		}
		sort(validation_network_vals.begin(), validation_network_vals.end());

		int best_start_index;
		int best_end_index;
		double best_val = numeric_limits<double>::lowest();
		for (int start_index = 0; start_index < (int)validation_network_vals.size(); start_index++) {
			for (int end_index = start_index; end_index < (int)validation_network_vals.size(); end_index++) {
				double curr_sum_vals = 0.0;
				/**
				 * - don't take before start_index
				 */
				for (int i_index = start_index; i_index < end_index; i_index++) {
					if (validation_network_vals[i_index].second.second == 1.0) {
						curr_sum_vals += validation_network_vals[i_index].second.first;
					}
				}
				for (int i_index = end_index; i_index < (int)validation_network_vals.size(); i_index++) {
					if (validation_network_vals[i_index].first > 0.0) {
						curr_sum_vals += validation_network_vals[i_index].second.first;
					}
				}

				if (curr_sum_vals > best_val) {
					best_start_index = start_index;
					best_end_index = end_index;
					best_val = curr_sum_vals;
				}
			}
		}

		double start_limit = validation_network_vals[best_start_index].first;
		double end_limit = validation_network_vals[best_end_index].first;

		cout << "best_start_index: " << best_start_index << endl;
		cout << "best_end_index: " << best_end_index << endl;
		cout << "start_limit: " << start_limit << endl;
		cout << "end_limit: " << end_limit << endl;

		// temp
		start_limit = -100.0;
		end_limit = 100.0;

		vector<vector<double>> test_obs;
		vector<double> test_vals;
		vector<double> test_underlying_vals;
		for (int i_index = 0; i_index < NUM_TEST_SAMPLES; i_index++) {
			double large_obs = rand()%3 - 1;
			double small_obs = rand()%3 - 1;
			double noise = noise_distribution(generator);
			double result = large_obs * LARGE_IMPACT + small_obs * SMALL_IMPACT + noise;

			vector<double> input;
			input.push_back(large_obs);
			input.push_back(small_obs);
			for (int ii_index = 0; ii_index < NUM_INPUTS-2; ii_index++) {
				input.push_back(rand()%3 - 1);
			}

			test_obs.push_back(input);
			test_vals.push_back(result);
			test_underlying_vals.push_back(large_obs * LARGE_IMPACT + small_obs * SMALL_IMPACT);
		}

		vector<double> test_network_vals(test_vals.size());
		for (int i_index = 0; i_index < (int)test_vals.size(); i_index++) {
			network->activate(test_obs[i_index]);
			test_network_vals[i_index] = network->output->acti_vals[0];
		}

		double true_network_sum_vals = 0.0;
		double true_network_sum_underlying_vals = 0.0;
		for (int i_index = 0; i_index < (int)test_vals.size(); i_index++) {
			if (test_network_vals[i_index] > 0.0) {
				true_network_sum_vals += test_vals[i_index];
				true_network_sum_underlying_vals += test_underlying_vals[i_index];
			}
		}
		cout << "true_network_sum_vals: " << true_network_sum_vals << endl;
		cout << "true_network_sum_underlying_vals: " << true_network_sum_underlying_vals << endl;

		double dual_sum_vals = 0.0;
		double dual_sum_underlying_vals = 0.0;
		for (int i_index = 0; i_index < (int)test_vals.size(); i_index++) {
			if (test_network_vals[i_index] >= end_limit
					&& test_network_vals[i_index] > 0.0) {
				dual_sum_vals += test_vals[i_index];
				dual_sum_underlying_vals += test_underlying_vals[i_index];
			} else if (test_network_vals[i_index] >= start_limit) {
				if (test_obs[i_index][1] == 1.0) {
					dual_sum_vals += test_vals[i_index];
					dual_sum_underlying_vals += test_underlying_vals[i_index];
				}
			}
		}
		cout << "dual_sum_vals: " << dual_sum_vals << endl;
		cout << "dual_sum_underlying_vals: " << dual_sum_underlying_vals << endl;

		if (true_network_sum_vals >= dual_sum_vals) {
			num_true_better++;
		} else {
			num_dual_better++;
		}
		cout << "num_true_better: " << num_true_better << endl;
		cout << "num_dual_better: " << num_dual_better << endl;

		if (true_network_sum_underlying_vals >= dual_sum_underlying_vals) {
			num_true_underlying_better++;
		} else {
			num_dual_underlying_better++;
		}
		cout << "num_true_underlying_better: " << num_true_underlying_better << endl;
		cout << "num_dual_underlying_better: " << num_dual_underlying_better << endl;

		cout << endl;

		delete network;
	}

	cout << "Done" << endl;
}
