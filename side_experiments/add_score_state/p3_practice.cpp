#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* initialization_network = new Network(1, 10, 1);
	Network* network = new Network(4, 100, 1);

	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
		if (epoch_index%100 == 0) {
			cout << endl;
			cout << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		if (epoch_index%100 == 0) {
			vector<AbstractNetworkHistory*> network_historys;

			double state = 0.0;

			int rand_non_empty = rand()%6;
			cout << "rand_non_empty: " << rand_non_empty << endl;

			vector<double> initialization_input{(double)rand_non_empty};
			initialization_network->activate(initialization_input);
			state = initialization_network->output->acti_vals[0];
			cout << "starting state: " << state << endl;

			int num_iters = rand()%6;

			int sum = 0;
			for (int i = 0; i < 6; i++) {
				int first_value = rand()%2;
				int second_value = rand()%2;
				cout << i << " first_value: " << first_value << endl;
				cout << i << " second_value: " << second_value << endl;

				if (i < rand_non_empty) {
					if (first_value == second_value) {
						sum += 2;
					}
				} else {
					if (i < num_iters) {
						sum -= 1;
					}
				}

				if (i < num_iters) {
					vector<double> input;
					input.push_back(state);
					input.push_back(rand_non_empty);
					input.push_back(first_value);
					input.push_back(second_value);
					network->activate(input, network_historys);
					state = network->output->acti_vals[0];
					cout << i << " state: " << state << endl;
				}
			}

			double state_error = sum - state;
			sum_error += abs(state_error);

			cout << "sum: " << sum << endl;
			cout << "state_error: " << state_error << endl;

			while (network_historys.size() > 0) {
				network_historys.back()->reset_weights();

				vector<double> errors{state_error};
				if (epoch_index < 40000) {
					network->backprop(errors, 0.01);
				} else {
					network->backprop(errors, 0.001);
				}
				state_error = network->input->errors[0];
				network->input->errors[0] = 0.0;

				delete network_historys.back();
				network_historys.pop_back();
			}

			vector<double> errors{state_error};
			if (epoch_index < 40000) {
				initialization_network->backprop(errors, 0.01);
			} else {
				initialization_network->backprop(errors, 0.001);
			}
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			vector<AbstractNetworkHistory*> network_historys;

			double state = 0.0;

			int rand_non_empty = rand()%6;

			vector<double> initialization_input{(double)rand_non_empty};
			initialization_network->activate(initialization_input);
			state = initialization_network->output->acti_vals[0];

			int num_iters = rand()%6;

			int sum = 0;
			for (int i = 0; i < 6; i++) {
				int first_value = rand()%2;
				int second_value = rand()%2;

				if (i < rand_non_empty) {
					if (first_value == second_value) {
						sum += 2;
					}
				} else {
					if (i < num_iters) {
						sum -= 1;
					}
				}

				if (i < num_iters) {
					vector<double> input;
					input.push_back(state);
					input.push_back(rand_non_empty);
					input.push_back(first_value);
					input.push_back(second_value);
					network->activate(input, network_historys);
					state = network->output->acti_vals[0];
				}
			}

			double state_error = sum - state;
			sum_error += abs(state_error);

			while (network_historys.size() > 0) {
				network_historys.back()->reset_weights();

				vector<double> errors{state_error};
				if (epoch_index < 40000) {
					network->backprop(errors, 0.01);
				} else {
					network->backprop(errors, 0.001);
				}
				state_error = network->input->errors[0];
				network->input->errors[0] = 0.0;

				delete network_historys.back();
				network_historys.pop_back();
			}

			vector<double> errors{state_error};
			if (epoch_index < 40000) {
				initialization_network->backprop(errors, 0.01);
			} else {
				initialization_network->backprop(errors, 0.001);
			}
		}
	}

	cout << "Done" << endl;
}
