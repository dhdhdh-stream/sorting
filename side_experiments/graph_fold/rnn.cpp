#include <iostream>
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

	vector<Network*> networks;
	for (int i = 0; i < 7; i++) {
		networks.push_back(new Network(9, 1000, 8));
	}
	networks.push_back(new Network(9, 1000, 1));

	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < 500000; epoch_index++) {
		if (epoch_index%100 == 0) {
			cout << endl;
			cout << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			double state[8] = {};

			int sum = 0;
			for (int i = 0; i < 7; i++) {
				vector<double> inputs;

				for (int s_index = 0; s_index < 8; s_index++) {
					inputs.push_back(state[s_index]);
				}

				if (rand()%2 == 0) {
					inputs.push_back(1.0);
					sum++;
				} else {
					inputs.push_back(-1.0);
				}

				networks[i]->activate(inputs);

				for (int s_index = 0; s_index < 8; s_index++) {
					state[s_index] += networks[i]->output->acti_vals[s_index];
				}
			}

			vector<double> inputs;

			for (int s_index = 0; s_index < 8; s_index++) {
				inputs.push_back(state[s_index]);
			}

			if (rand()%2 == 0) {
				inputs.push_back(1.0);
				sum++;
			} else {
				inputs.push_back(-1.0);
			}

			networks[7]->activate(inputs);

			bool is_even = (sum%2 == 0);

			vector<double> errors;
			if (is_even) {
				if (networks[7]->output->acti_vals[0] > 1.0) {
					errors.push_back(0.0);
				} else {
					errors.push_back(1.0 - networks[7]->output->acti_vals[0]);
				}
			} else {
				if (networks[7]->output->acti_vals[0] < 0.0) {
					errors.push_back(0.0);
				} else {
					errors.push_back(0.0 - networks[7]->output->acti_vals[0]);
				}
			}
			sum_error += abs(errors[0]);
			networks[7]->backprop(errors);

			double state_errors[8] = {};
			for (int s_index = 0; s_index < 8; s_index++) {
				state_errors[s_index] += networks[7]->input->errors[s_index];
				networks[7]->input->errors[s_index] = 0.0;
			}

			for (int i = 6; i >= 0; i--) {
				vector<double> errors;
				for (int s_index = 0; s_index < 8; s_index++) {
					errors.push_back(state_errors[s_index]);
				}

				networks[i]->backprop(errors);

				for (int s_index = 0; s_index < 8; s_index++) {
					state_errors[s_index] += networks[i]->input->errors[s_index];
					networks[i]->input->errors[s_index] = 0.0;
				}
			}
		}
	}

	cout << "Done" << endl;
}
