// - compare training 1 network on combined signal vs. training multiple networks then summing

// - double counts signal
//   - is that a problem?
//   - yes, let's say signal is good, but there's mildly bad outer impact

// - maybe train from inner to out
//   - minus inner impact from outer

// try strict hierarchy of scopes?
// - no, can't combine then?

// - doesn't work
//   - issue is that noise messes up outer which messes up overall
//   - so summing is not good because outer dominates
//     - has to be about ignoring
//       - possibly with filtering out strong results

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_INPUTS = 10;

const int NUM_TRAIN_SAMPLES = 1000;
// const int NUM_TEST_SAMPLES = 200;
const int NUM_TEST_SAMPLES = 1000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	int num_dual_better = 0;
	int num_single_better = 0;

	uniform_int_distribution<int> noise_distribution(-100, 100);
	uniform_int_distribution<int> large_distribution(-100, 100);
	uniform_int_distribution<int> small_distribution(-10, 10);
	uniform_int_distribution<int> train_distribution(0, NUM_TRAIN_SAMPLES-1);
	for (int epoch_iter = 0; epoch_iter < 1000; epoch_iter++) {
		vector<vector<double>> train_obs;
		vector<double> train_vals;
		for (int i_index = 0; i_index < NUM_TRAIN_SAMPLES; i_index++) {
			double large_obs = large_distribution(generator);
			double small_obs = small_distribution(generator);
			double noise = noise_distribution(generator);
			double result = large_obs + small_obs + noise;

			vector<double> input;
			input.push_back(large_obs);
			input.push_back(small_obs);
			for (int ii_index = 0; ii_index < NUM_INPUTS-2; ii_index++) {
				input.push_back(rand()%3 - 1);
			}

			train_obs.push_back(input);
			train_vals.push_back(result);
		}

		Network* signal_network = new Network(NUM_INPUTS,
											  NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			signal_network->activate(train_obs[rand_index]);

			double error = train_obs[rand_index][1] - signal_network->output->acti_vals[0];

			signal_network->backprop(error);
		}

		vector<double> signal_network_vals(train_obs.size());
		for (int h_index = 0; h_index < (int)train_obs.size(); h_index++) {
			signal_network->activate(train_obs[h_index]);
			signal_network_vals[h_index] = signal_network->output->acti_vals[0];
		}

		vector<double> mod_vals(train_vals.size());
		for (int h_index = 0; h_index < (int)train_vals.size(); h_index++) {
			mod_vals[h_index] = train_vals[h_index] - signal_network_vals[h_index];
		}

		Network* outer_network = new Network(NUM_INPUTS,
											 NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			outer_network->activate(train_obs[rand_index]);

			double error = mod_vals[rand_index] - outer_network->output->acti_vals[0];

			outer_network->backprop(error);
		}

		Network* single_network = new Network(NUM_INPUTS,
											  NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			single_network->activate(train_obs[rand_index]);

			double error = train_vals[rand_index] - single_network->output->acti_vals[0];

			single_network->backprop(error);
		}

		vector<vector<double>> test_obs;
		vector<double> test_vals;
		for (int i_index = 0; i_index < NUM_TEST_SAMPLES; i_index++) {
			double large_obs = large_distribution(generator);
			double small_obs = small_distribution(generator);
			double noise = noise_distribution(generator);
			double result = large_obs + small_obs + noise;

			vector<double> input;
			input.push_back(large_obs);
			input.push_back(small_obs);
			for (int ii_index = 0; ii_index < NUM_INPUTS-2; ii_index++) {
				input.push_back(rand()%3 - 1);
			}

			test_obs.push_back(input);
			test_vals.push_back(result);
		}

		double dual_sum_misguess = 0.0;
		double single_sum_misguess = 0.0;
		for (int i_index = 0; i_index < NUM_TEST_SAMPLES; i_index++) {
			signal_network->activate(test_obs[i_index]);
			outer_network->activate(test_obs[i_index]);
			single_network->activate(test_obs[i_index]);

			dual_sum_misguess += (test_vals[i_index] - signal_network->output->acti_vals[0] - outer_network->output->acti_vals[0])
				* (test_vals[i_index] - signal_network->output->acti_vals[0] - outer_network->output->acti_vals[0]);
			single_sum_misguess += (test_vals[i_index] - single_network->output->acti_vals[0])
				* (test_vals[i_index] - single_network->output->acti_vals[0]);
		}

		if (dual_sum_misguess < single_sum_misguess) {
			num_dual_better++;
		} else {
			num_single_better++;
		}
		cout << "num_dual_better: " << num_dual_better << endl;
		cout << "num_single_better: " << num_single_better << endl;
		cout << endl;

		delete signal_network;
		delete outer_network;
		delete single_network;
	}

	cout << "Done" << endl;
}
