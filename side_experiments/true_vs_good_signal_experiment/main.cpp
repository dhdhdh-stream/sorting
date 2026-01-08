// - around 2-to-1
//   - of course, without noise, would be 100-to-0
//     - but can never truly know without noise

// TODO: experiment with different sample sizes, noise, etc.

// - lowering train samples make signals better
// - lowering test samples make being better less relavent

// - a signal that never starts misguiding is true

// - always remove signals that can mislead?
//   - but allow noise/variance
//     - what's the difference?
//       - just sample size?
//         - but sample size of sample size?
//         - so across multiple paths/tries, mislead on average?

// - then are there any signals to find?
//   - maybe getting maxed is OK, but remove if start misleading

// - the best signals can prevent all other bad actions from being taken
//   - but most signals can't be that
//     - so there's always a chance that chasing a signal trips up something else
//       - so how to determine what is and isn't misleading?
//         - TODO: need to come up with a number

// - let's say that a random path can mess things up badly 50% of the time
//   - train on true will simply say no to the path
//     - happens 50% of the time
//   - train on signal won't realize that it should say no
//     - happens 50% of the time
//   - so what is misleading?
//     - when it goes above 50%?
//       - 50% chance of messing up + extra chance of mislead
// TODO: how to test this?

// - a path can be fully bad, but have a chance to succeed
//   - can be caught by true going to 0%
//   - not avoidable by signal
// - vs. there's a chance when the path should be performed vs. not
//   - good and true will choose when to do what
//     - same with signal
// - both will be found by explore
// - so failure rate for true is chance of path being fully bad
//   - failure is when experiment fails to yield positive result
// - failure rate for signal is chance of path being fully bad + chance of mislead
//   - mislead is: possibility of making bad decisions
//     - true cannot fail this way, but signal now can

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

// const int NUM_TRAIN_SAMPLES = 1000;
const int NUM_TRAIN_SAMPLES = 200;
const int NUM_TEST_SAMPLES = 200;
// const int NUM_TEST_SAMPLES = 20;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	int num_true_better = 0;
	int num_signal_better = 0;

	for (int epoch_iter = 0; epoch_iter < 1000; epoch_iter++) {
		uniform_int_distribution<int> obs_distribution(-10, 10);
		uniform_int_distribution<int> noise_distribution(-100, 100);

		vector<vector<double>> train_obs;
		vector<double> train_signal_vals;
		vector<double> train_true_vals;
		for (int i_index = 0; i_index < NUM_TRAIN_SAMPLES; i_index++) {
			double obs = obs_distribution(generator);
			vector<double> curr_obs{obs};

			int noise = noise_distribution(generator);

			train_obs.push_back(curr_obs);
			train_signal_vals.push_back(curr_obs[0]);
			train_true_vals.push_back(curr_obs[0] + noise);
		}

		uniform_int_distribution<int> train_distribution(0, NUM_TRAIN_SAMPLES-1);

		Network* true_network = new Network(1,
											NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			true_network->activate(train_obs[rand_index]);

			double error = train_true_vals[rand_index] - true_network->output->acti_vals[0];

			true_network->backprop(error);
		}

		Network* signal_network = new Network(1,
											  NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			signal_network->activate(train_obs[rand_index]);

			double error = train_signal_vals[rand_index] - signal_network->output->acti_vals[0];

			signal_network->backprop(error);
		}

		vector<vector<double>> test_obs;
		vector<double> test_true_vals;
		for (int i_index = 0; i_index < NUM_TEST_SAMPLES; i_index++) {
			double obs = obs_distribution(generator);
			vector<double> curr_obs{obs};

			int noise = noise_distribution(generator);

			test_obs.push_back(curr_obs);
			test_true_vals.push_back(curr_obs[0] + noise);
		}

		double sum_true_error = 0.0;
		double sum_signal_error = 0.0;
		for (int i_index = 0; i_index < NUM_TEST_SAMPLES; i_index++) {
			true_network->activate(test_obs[i_index]);
			double true_predict = true_network->output->acti_vals[0];
			sum_true_error += (true_predict - test_true_vals[i_index]) * (true_predict - test_true_vals[i_index]);

			signal_network->activate(test_obs[i_index]);
			double signal_predict = signal_network->output->acti_vals[0];
			sum_signal_error += (signal_predict - test_true_vals[i_index]) * (signal_predict - test_true_vals[i_index]);
		}

		cout << "sum_true_error: " << sum_true_error << endl;
		cout << "sum_signal_error: " << sum_signal_error << endl;
		if (sum_true_error <= sum_signal_error) {
			num_true_better++;
		} else {
			num_signal_better++;
		}
		cout << "num_true_better: " << num_true_better << endl;
		cout << "num_signal_better: " << num_signal_better << endl;

		cout << endl;

		delete true_network;
		delete signal_network;
	}

	cout << "Done" << endl;
}
