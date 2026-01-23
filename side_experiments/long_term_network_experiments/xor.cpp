#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_INPUTS = 20;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	uniform_int_distribution<int> input_distribution(0, 1);

	Network* network = new Network(NUM_INPUTS);

	// for (int iter_index = 0; iter_index < 300000; iter_index++) {
	for (int iter_index = 0; iter_index < 100000; iter_index++) {
		vector<double> inputs(NUM_INPUTS);
		for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
			inputs[i_index] = input_distribution(generator);
		}

		double result;
		if (inputs[0] == inputs[1]) {
			result = 1.0;
		} else {
			result = -1.0;
		}

		network->activate(inputs);

		double error = result - network->output->acti_vals[0];

		network->backprop(error);

		if (iter_index % 100000 == 0) {
			cout << iter_index << endl;
		}
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		vector<double> inputs(NUM_INPUTS);
		for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
			inputs[i_index] = input_distribution(generator);
		}

		network->activate(inputs);

		cout << "inputs[0]: " << inputs[0] << endl;
		cout << "inputs[1]: " << inputs[1] << endl;
		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	}

	delete network;

	cout << "Done" << endl;
}
