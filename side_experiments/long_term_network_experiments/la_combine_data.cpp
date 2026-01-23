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

	// for (int epoch_index = 0; epoch_index < 20; epoch_index++) {
	for (int epoch_index = 0; epoch_index < 10; epoch_index++) {
		int existing_weight;
		if (epoch_index > 10) {
			existing_weight = 10;
		} else {
			existing_weight = epoch_index;
		}
		uniform_int_distribution<int> new_distribution(0, existing_weight);

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

		vector<double> existing_vals(NUM_TRAIN_DATA);
		for (int t_index = 0; t_index < NUM_TRAIN_DATA; t_index++) {
			network->activate(train_data[t_index]);
			existing_vals[t_index] = network->output->acti_vals[0];
		}

		// int train_iters;
		// if (epoch_index >= 10) {
		// 	train_iters = 100000;
		// } else {
		// 	train_iters = 10000 * (epoch_index + 1);
		// }

		// for (int iter_index = 0; iter_index < train_iters; iter_index++) {
		// 	int rand_index = train_distribution(generator);

		// 	network->activate(train_data[rand_index]);

		// 	double error;
		// 	if (new_distribution(generator) == 0) {
		// 		error = train_targets[rand_index] - network->output->acti_vals[0];
		// 	} else {
		// 		error = existing_vals[rand_index] - network->output->acti_vals[0];
		// 	}

		// 	network->backprop(error);

		// 	if (iter_index % 100000 == 0) {
		// 		cout << iter_index << endl;
		// 	}
		// }

		vector<double> adjusted_targets(NUM_TRAIN_DATA);
		for (int t_index = 0; t_index < NUM_TRAIN_DATA; t_index++) {
			if (epoch_index >= 9) {
				adjusted_targets[t_index] = (9.0 * existing_vals[t_index] + train_targets[t_index]) / 10.0;
			} else {
				adjusted_targets[t_index] = (epoch_index * existing_vals[t_index] + train_targets[t_index]) / (1.0 + epoch_index);
			}
		}

		for (int iter_index = 0; iter_index < 10000; iter_index++) {
			int rand_index = train_distribution(generator);

			network->activate(train_data[rand_index]);

			double error = adjusted_targets[rand_index] - network->output->acti_vals[0];

			network->backprop(error);

			if (iter_index % 100000 == 0) {
				cout << iter_index << endl;
			}
		}
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		vector<double> inputs(NUM_INPUTS);
		for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
			inputs[i_index] = input_distribution(generator);
		}

		network->activate(inputs);

		cout << "inputs[0]: " << inputs[0] << endl;
		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	}

	double sum_misguess = 0.0;
	for (int i_index = 0; i_index < 1000; i_index++) {
		vector<double> inputs(NUM_INPUTS);
		for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
			inputs[i_index] = input_distribution(generator);
		}

		network->activate(inputs);

		sum_misguess += (inputs[0] - network->output->acti_vals[0]) * (inputs[0] - network->output->acti_vals[0]);
	}
	cout << "sum_misguess: " << sum_misguess << endl;

	delete network;

	cout << "Done" << endl;
}
