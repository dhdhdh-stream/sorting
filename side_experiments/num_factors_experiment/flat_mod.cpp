// - need large network size to memorize

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
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

	Network* network = new Network(LEAKY_LAYER, 1, 1);

	// geometric_distribution<int> val_distribution(0.3);
	uniform_int_distribution<int> val_distribution(0, 4);
	// for (int iter_index = 0; iter_index < 300000; iter_index++) {
	for (int iter_index = 0; iter_index < 1000000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int val = val_distribution(generator);
		vector<double> inputs{(double)val};
		network->activate(inputs);

		double target_val;
		if (val % 2 == 0) {
			target_val = 1.0;
		} else {
			target_val = -1.0;
		}
		vector<double> errors{target_val - network->output->acti_vals[0]};
		network->backprop(errors);

		if (iter_index % 20 == 0) {
			network->update();
		}
	}

	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		int val = val_distribution(generator);
		cout << "val: " << val << endl;
		vector<double> inputs{(double)val};
		network->activate(inputs);

		double target_val;
		if (val % 2 == 0) {
			target_val = 1.0;
		} else {
			target_val = -1.0;
		}

		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
		cout << "target_val: " << target_val << endl;
	}

	cout << "Done" << endl;
}
