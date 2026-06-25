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

	Network* network = new Network(1);
	double hidden_1_average_max_update = 0.0;
	double hidden_2_average_max_update = 0.0;
	double hidden_3_average_max_update = 0.0;
	double output_average_max_update = 0.0;

	uniform_int_distribution<int> base_distribution(0, 1);
	uniform_int_distribution<int> noise_distribution(-10, 10);
	uniform_int_distribution<int> output_distribution(0, 1);
	for (int iter_index = 0; iter_index < 10000000; iter_index++) {
		int base = base_distribution(generator);
		double val = 5.0 * base + noise_distribution(generator);

		vector<double> inputs{val};
		network->activate(inputs);

		double target_val;
		if (output_distribution(generator) == 0) {
			target_val = base + 1.0;
		} else {
			target_val = base - 1.0;
		}

		double error = target_val - network->output->acti_vals[0];

		// network->backprop(error);
		network->init_backprop(error,
							   hidden_1_average_max_update,
							   hidden_2_average_max_update,
							   hidden_3_average_max_update,
							   output_average_max_update);

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;
			cout << "val: " << val << endl;
			cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
			cout << "target_val: " << target_val << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
