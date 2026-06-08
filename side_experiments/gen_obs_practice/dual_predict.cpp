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

	vector<Network*> networks;
	networks.push_back(new Network(0));
	networks.push_back(new Network(0));

	// uniform_int_distribution<int> target_distribution(0, 1);
	uniform_int_distribution<int> target_distribution(0, 9);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		vector<double> predicted(networks.size());
		vector<double> inputs;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			networks[n_index]->activate(inputs);
			predicted[n_index] = networks[n_index]->output->acti_vals[0];
		}

		double target_val = target_distribution(generator);

		double min_error = numeric_limits<double>::max();
		int min_index;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			double curr_error = (target_val - predicted[n_index]) * (target_val - predicted[n_index]);
			if (curr_error < min_error) {
				min_error = curr_error;
				min_index = n_index;
			}
		}

		double error = target_val - networks[min_index]->output->acti_vals[0];
		networks[min_index]->backprop(error);

		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
			cout << "predicted[0]: " << predicted[0] << endl;
			cout << "predicted[1]: " << predicted[1] << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
