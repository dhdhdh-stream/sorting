// - with infinite training samples, network correct with just true
// - but that changes when samples are limited

// - use test samples to pick a range where mistakes are made?

// - maybe try different confidences?
//   - use test/validation samples to help determine?
// - too much noise to use things like standard deviation to determine what's optimal/should be done
// - true network also not good enough to have clean confidence limits?

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_INPUTS = 10;

const int NUM_TRAIN_SAMPLES = 1000;
const int NUM_TEST_SAMPLES = 200;

// const double CONFIDENCE_LIMIT = 8.0;
const double CONFIDENCE_LIMIT = 2.0;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	uniform_int_distribution<int> noise_distribution(-100, 100);

	vector<vector<double>> train_obs;
	vector<double> train_vals;
	for (int i_index = 0; i_index < NUM_TRAIN_SAMPLES; i_index++) {
		double large_obs = rand()%3 - 1;
		double small_obs = rand()%3 - 1;
		double noise = noise_distribution(generator);
		double result = large_obs * 10.0 + small_obs * 1.0 + noise;

		vector<double> input;
		input.push_back(large_obs);
		input.push_back(small_obs);
		for (int ii_index = 0; ii_index < NUM_INPUTS-2; ii_index++) {
			input.push_back(rand()%3 - 1);
		}

		train_obs.push_back(input);
		train_vals.push_back(result);
	}

	double sum_vals = 0.0;
	for (int i_index = 0; i_index < (int)train_vals.size(); i_index++) {
		sum_vals += train_vals[i_index];
	}
	double val_average = sum_vals / (double)train_vals.size();

	double sum_variance = 0.0;
	for (int i_index = 0; i_index < (int)train_vals.size(); i_index++) {
		sum_variance += (train_vals[i_index] - val_average) * (train_vals[i_index] - val_average);
	}
	double val_standard_deviation = sqrt(sum_variance / (double)train_vals.size());

	cout << "val_average: " << val_average << endl;
	cout << "val_standard_deviation: " << val_standard_deviation << endl;

	Network* network = new Network(NUM_INPUTS,
								   NETWORK_SIZE_SMALL);
	uniform_int_distribution<int> train_distribution(0, NUM_TRAIN_SAMPLES-1);
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		int rand_index = train_distribution(generator);

		network->activate(train_obs[rand_index]);

		double error = train_vals[rand_index] - network->output->acti_vals[0];

		network->backprop(error);
	}

	vector<double> network_vals(train_vals.size());
	for (int i_index = 0; i_index < (int)train_vals.size(); i_index++) {
		network->activate(train_obs[i_index]);
		network_vals[i_index] = network->output->acti_vals[0];
	}

	double sum_network_vals = 0.0;
	double sum_network_absolute_vals = 0.0;
	for (int i_index = 0; i_index < (int)network_vals.size(); i_index++) {
		sum_network_vals += network_vals[i_index];
		sum_network_absolute_vals += abs(network_vals[i_index]);
	}
	double network_val_average = sum_network_vals / (double)network_vals.size();
	double network_absolute_val_average = sum_network_absolute_vals / (double)network_vals.size();

	double sum_network_variance = 0.0;
	for (int i_index = 0; i_index < (int)network_vals.size(); i_index++) {
		sum_network_variance += (network_vals[i_index] - network_val_average) * (network_vals[i_index] - network_val_average);
	}
	double network_val_standard_deviation = sqrt(sum_network_variance / (double)network_vals.size());

	cout << "network_absolute_val_average: " << network_absolute_val_average << endl;
	cout << "network_val_standard_deviation: " << network_val_standard_deviation << endl;

	vector<vector<double>> test_obs;
	vector<double> test_vals;
	for (int i_index = 0; i_index < NUM_TEST_SAMPLES; i_index++) {
		double large_obs = rand()%3 - 1;
		double small_obs = rand()%3 - 1;
		double noise = noise_distribution(generator);
		double result = large_obs * 10.0 + small_obs * 1.0 + noise;

		vector<double> input;
		input.push_back(large_obs);
		input.push_back(small_obs);
		for (int ii_index = 0; ii_index < NUM_INPUTS-2; ii_index++) {
			input.push_back(rand()%3 - 1);
		}

		test_obs.push_back(input);
		test_vals.push_back(result);
	}

	vector<double> test_network_vals(test_vals.size());
	for (int i_index = 0; i_index < (int)test_vals.size(); i_index++) {
		network->activate(test_obs[i_index]);
		test_network_vals[i_index] = network->output->acti_vals[0];
	}

	int num_correct = 0.0;
	int num_incorrect = 0.0;
	for (int i_index = 0; i_index < (int)test_vals.size(); i_index++) {
		if (test_network_vals[i_index] >= CONFIDENCE_LIMIT) {
			if (test_vals[i_index] < 0.0) {
				num_incorrect++;
			} else {
				num_correct++;
			}
		} else if (test_network_vals[i_index] <= -CONFIDENCE_LIMIT) {
			if (test_vals[i_index] > 0.0) {
				num_incorrect++;
			} else {
				num_correct++;
			}
		}
	}

	cout << "num_correct: " << num_correct << endl;
	cout << "num_incorrect: " << num_incorrect << endl;

	for (int i_index = 0; i_index < 20; i_index++) {
		double large_obs = rand()%3 - 1;
		double small_obs = rand()%3 - 1;
		double noise = noise_distribution(generator);
		double result = large_obs * 10.0 + small_obs * 1.0 + noise;

		vector<double> input;
		input.push_back(large_obs);
		input.push_back(small_obs);
		for (int ii_index = 0; ii_index < NUM_INPUTS-2; ii_index++) {
			input.push_back(rand()%3 - 1);
		}
		network->activate(input);

		cout << "large_obs: " << large_obs << endl;
		cout << "small_obs: " << small_obs << endl;
		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
		cout << "result: " << result << endl;
		cout << endl;
	}

	delete network;

	cout << "Done" << endl;
}
