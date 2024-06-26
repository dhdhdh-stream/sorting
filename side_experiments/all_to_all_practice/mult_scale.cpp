#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "mult_scale_network.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	MultScaleNetwork* network = new MultScaleNetwork();

	vector<double> sum_errors(2, 0.0);
	for (int iter_index = 0; iter_index < 1000000; iter_index++) {
		vector<double> inputs(2);
		for (int i = 0; i < 2; i++) {
			inputs[i] = rand()%11 - 5;
		}

		vector<double> targets(2);
		targets[0] = 300 * inputs[0] - 100 * inputs[1] + rand()%1001 - 500;
		targets[1] = -0.02 * inputs[0] + 0.05 * inputs[1] + ((rand()%1001 - 500) / 10000.0);
		// targets[1] = -0.03 * inputs[0] + 0.01 * inputs[1] + ((rand()%1001 - 500) / 10000.0);

		network->activate(inputs);
		vector<double> errors(2);
		for (int i = 0; i < 2; i++) {
			sum_errors[i] = abs(targets[i] - network->output->acti_vals[i]);

			errors[i] = targets[i] - network->output->acti_vals[i];
		}
		// errors[0] /= 10000.0;
		// errors[1] *= 10000.0;
		network->backprop(errors);

		if (iter_index%10000 == 0) {
			cout << "inputs:" << endl;
			for (int i = 0; i < 2; i++) {
				cout << i << ": " << inputs[i] << endl;
			}

			cout << "targets:" << endl;
			for (int i = 0; i < 2; i++) {
				cout << i << ": " << targets[i] << endl;
			}
			cout << "underlying:" << endl;
			cout << "0: " << 300 * inputs[0] - 100 * inputs[1] << endl;
			cout << "1: " << -0.02 * inputs[0] + 0.05 * inputs[1] << endl;
			// cout << "1: " << -0.03 * inputs[0] + 0.01 * inputs[1] << endl;
			cout << "predicted:" << endl;
			for (int i = 0; i < 2; i++) {
				cout << i << ": " << network->output->acti_vals[i] << endl;
			}

			cout << "sum_errors:" << endl;
			for (int i = 0; i < 2; i++) {
				cout << i << ": " << sum_errors[i] << endl;
			}

			// cout << "output_average_errors:" << endl;
			// for (int i = 0; i < 2; i++) {
			// 	cout << i << ": " << network->output_average_errors[i] << endl;
			// }

			cout << endl;

			for (int i = 0; i < 2; i++) {
				sum_errors[i] = 0.0;
			}
		}
	}

	delete network;

	cout << "Done" << endl;
}
