// - it is better to use Eigen
//   - though not by that much
//     - on a 100+ node network, 2-3x speedup

// - optimization flags make a huge difference though
//   - 50x speedup
//   - -O2 better than -O3(?)

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "eigen_network.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	EigenNetwork* network = new EigenNetwork(2, 1);

	auto start_time = chrono::high_resolution_clock::now();

	uniform_int_distribution<int> distribution(0, 1);
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		double val_1 = distribution(generator);
		double val_2 = distribution(generator);

		double target_val;
		if (val_1 == val_2) {
			target_val = 1.0;
		} else {
			target_val = -1.0;
		}

		vector<double> inputs{val_1, val_2};
		network->activate(inputs);

		vector<double> errors{target_val - network->output->acti_vals(0)};
		network->backprop(errors);

		if ((iter_index + 1) % 10000 == 0) {
			cout << iter_index << endl;
			cout << "val_1: " << val_1 << endl;
			cout << "val_2: " << val_2 << endl;
			cout << "network->output->acti_vals(0): " << network->output->acti_vals(0) << endl;
			cout << "target_val: " << target_val << endl;
			cout << endl;
		}
	}

	auto curr_time = chrono::high_resolution_clock::now();
	auto time_diff = chrono::duration_cast<chrono::milliseconds>(curr_time - start_time);
	cout << "time_diff.count(): " << time_diff.count() << endl;

	cout << "Done" << endl;
}
