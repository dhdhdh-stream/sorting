#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

default_random_engine generator;

const int NUM_STEPS = 10;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* network = new Network(2);

	uniform_int_distribution<int> add_distribution(0, 4);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << "iter_index: " << iter_index << endl;
		}

		vector<vector<double>> inputs;

		double val = 0.0;
		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			vector<double> curr_inputs;

			curr_inputs.push_back(val);

			val += add_distribution(generator);

			curr_inputs.push_back(val);

			inputs.push_back(curr_inputs);
		}

		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			network->activate(inputs[s_index]);

			double error = val - network->output->acti_vals[0];

			network->backprop(error);
		}
	}

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		double val = 0.0;
		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			vector<double> curr_inputs;

			double start_val = val;
			curr_inputs.push_back(val);

			val += add_distribution(generator);

			double end_val = val;
			curr_inputs.push_back(val);

			network->activate(curr_inputs);

			cout << "start_val: " << start_val << endl;
			cout << "end_val: " << end_val << endl;
			cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
