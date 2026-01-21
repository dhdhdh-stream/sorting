#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* network = new Network(2);

	uniform_int_distribution<int> input_distribution(-10, 10);
	uniform_int_distribution<int> noise_distribution(-10, 10);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		double val_1 = input_distribution(generator);
		double val_2 = input_distribution(generator);
		double noise = noise_distribution(generator);

		double result = val_1 + val_2 + noise;

		vector<double> inputs{val_1, val_2};
		network->activate(inputs);

		double error = result - network->output->acti_vals[0];

		network->backprop(error);
	}

	cout << "pre:" << endl;
	for (int i_index = 0; i_index < 10; i_index++) {
		double val_1 = input_distribution(generator);
		double val_2 = input_distribution(generator);

		vector<double> inputs{val_1, val_2};

		network->activate(inputs);

		cout << "val_1: " << val_1 << endl;
		cout << "val_2: " << val_2 << endl;
		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	}

	Network* original_network = new Network(network);

	uniform_int_distribution<int> is_original_distribution(0, 1);
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		double val_1 = input_distribution(generator);
		double val_2 = input_distribution(generator);
		double noise = noise_distribution(generator);

		vector<double> inputs{val_1, val_2};

		double result;
		if (is_original_distribution(generator) == 0) {
			original_network->activate(inputs);
			result = original_network->output->acti_vals[0];
		} else {
			result = val_1 - val_2 + noise;
		}

		network->activate(inputs);

		double error = result - network->output->acti_vals[0];

		network->backprop(error);
	}

	cout << "post:" << endl;
	for (int i_index = 0; i_index < 10; i_index++) {
		double val_1 = input_distribution(generator);
		double val_2 = input_distribution(generator);

		vector<double> inputs{val_1, val_2};

		network->activate(inputs);

		cout << "val_1: " << val_1 << endl;
		cout << "val_2: " << val_2 << endl;
		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	}

	delete network;
	delete original_network;

	cout << "Done" << endl;
}
