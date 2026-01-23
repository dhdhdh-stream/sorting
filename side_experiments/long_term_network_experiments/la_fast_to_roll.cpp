#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_TRAIN_DATA = 1000;
const int NUM_INPUTS = 20;

// const int NEW_SAMPLES_PER_EPOCH = 100;
const int NEW_SAMPLES_PER_EPOCH = 10;
// const int ITERS_PER_EPOCH = 1000;
const int ITERS_PER_EPOCH = 100;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	uniform_int_distribution<int> input_distribution(-1, 1);
	uniform_int_distribution<int> noise_distribution(-10, 10);

	uniform_int_distribution<int> train_distribution(0, NUM_TRAIN_DATA-1);

	Network* network = new Network(NUM_INPUTS);

	vector<vector<double>> train_data(NUM_TRAIN_DATA);
	vector<double> train_targets(NUM_TRAIN_DATA);
	for (int t_index = 0; t_index < NUM_TRAIN_DATA; t_index++) {
		vector<double> inputs(NUM_INPUTS);
		for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
			inputs[i_index] = input_distribution(generator);
		}
		double noise = noise_distribution(generator);

		double result = inputs[0] + noise;

		train_data[t_index] = inputs;
		train_targets[t_index] = result;
	}

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		int rand_index = train_distribution(generator);

		network->activate(train_data[rand_index]);

		double error = train_targets[rand_index] - network->output->acti_vals[0];

		network->backprop(error);
	}

	{
		double sum_misguess = 0.0;
		for (int i_index = 0; i_index < 1000; i_index++) {
			vector<double> inputs(NUM_INPUTS);
			for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
				inputs[i_index] = input_distribution(generator);
			}

			network->activate(inputs);

			sum_misguess += (inputs[0] - network->output->acti_vals[0]) * (inputs[0] - network->output->acti_vals[0]);
		}
		cout << "pre sum_misguess: " << sum_misguess << endl;
	}

	int long_index = 0;

	// for (int epoch_index = 0; epoch_index < 100; epoch_index++) {
	for (int epoch_index = 0; epoch_index < 1000; epoch_index++) {
		for (int t_index = 0; t_index < NEW_SAMPLES_PER_EPOCH; t_index++) {
			vector<double> inputs(NUM_INPUTS);
			for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
				inputs[i_index] = input_distribution(generator);
			}
			double noise = noise_distribution(generator);

			double result = inputs[0] + noise;

			train_data[long_index] = inputs;
			train_targets[long_index] = result;

			long_index++;
			if (long_index >= NUM_TRAIN_DATA) {
				long_index = 0;
			}
		}

		for (int iter_index = 0; iter_index < ITERS_PER_EPOCH; iter_index++) {
			int rand_index = train_distribution(generator);

			network->activate(train_data[rand_index]);

			double error = train_targets[rand_index] - network->output->acti_vals[0];

			network->backprop(error);
		}
	}

	// for (int i_index = 0; i_index < 10; i_index++) {
	// 	vector<double> inputs(NUM_INPUTS);
	// 	for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
	// 		inputs[i_index] = input_distribution(generator);
	// 	}

	// 	network->activate(inputs);

	// 	cout << "inputs[0]: " << inputs[0] << endl;
	// 	cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	// }

	{
		double sum_misguess = 0.0;
		for (int i_index = 0; i_index < 1000; i_index++) {
			vector<double> inputs(NUM_INPUTS);
			for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
				inputs[i_index] = input_distribution(generator);
			}

			network->activate(inputs);

			sum_misguess += (inputs[0] - network->output->acti_vals[0]) * (inputs[0] - network->output->acti_vals[0]);
		}
		cout << "post sum_misguess: " << sum_misguess << endl;
	}

	delete network;

	cout << "Done" << endl;
}
