#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network network(8, 1000, 1);

	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
		if (epoch_index%1000 == 0) {
			cout << endl;
			cout << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			int sum = 0;
			vector<double> inputs;
			for (int i = 0; i < 8; i++) {
				if (rand()%2 == 0) {
					inputs.push_back(1.0);
					sum++;
				} else {
					inputs.push_back(-1.0);
				}
			}
			bool is_even = (sum%2 == 0);

			network.activate(inputs);

			vector<double> errors;
			if (is_even) {
				if (network.output->acti_vals[0] > 1.0) {
					errors.push_back(0.0);
				} else {
					errors.push_back(1.0 - network.output->acti_vals[0]);
				}
			} else {
				if (network.output->acti_vals[0] < 0.0) {
					errors.push_back(0.0);
				} else {
					errors.push_back(0.0 - network.output->acti_vals[0]);
				}
			}
			sum_error += abs(errors[0]);
			network.backprop(errors);
		}
	}

	cout << "Done" << endl;
}
