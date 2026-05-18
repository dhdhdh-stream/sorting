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

	Network* branch_1_network = new Network(LEAKY_LAYER, 2, 1);
	int branch_1_epoch = 0;
	Network* branch_2_network = new Network(LEAKY_LAYER, 2, 1);
	int branch_2_epoch = 0;

	uniform_int_distribution<int> input_distribution(0, 1);
	uniform_int_distribution<int> noise_distribution(-5, 5);

	for (int iter_index = 0; iter_index < 500000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		double input_1 = input_distribution(generator);
		double input_2 = input_distribution(generator);
		vector<double> inputs{input_1, input_2};

		branch_1_network->activate(inputs);
		double branch_1_predicted = branch_1_network->output->acti_vals[0];
		branch_2_network->activate(inputs);
		double branch_2_predicted = branch_2_network->output->acti_vals[0];

		if (branch_1_predicted > branch_2_predicted) {
			double target_val;
			if (input_1 == input_2) {
				target_val = 1.0 + noise_distribution(generator);
			} else {
				target_val = -1.0 + noise_distribution(generator);
			}

			vector<double> errors{target_val - branch_1_predicted};
			branch_1_network->backprop(errors);

			branch_1_epoch++;
			if (branch_1_epoch >= 20) {
				branch_1_network->update();

				branch_1_epoch = 0;
			}
		} else {
			double target_val;
			if (input_1 == input_2) {
				target_val = -1.0 + noise_distribution(generator);
			} else {
				target_val = 1.0 + noise_distribution(generator);
			}

			vector<double> errors{target_val - branch_2_predicted};
			branch_2_network->backprop(errors);

			branch_2_epoch++;
			if (branch_2_epoch >= 20) {
				branch_2_network->update();

				branch_2_epoch = 0;
			}
		}
	}

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		cout << "pre " << iter_index << endl;

		double input_1 = input_distribution(generator);
		cout << "input_1: " << input_1 << endl;
		double input_2 = input_distribution(generator);
		cout << "input_2: " << input_2 << endl;
		vector<double> inputs{input_1, input_2};

		branch_1_network->activate(inputs);
		double branch_1_predicted = branch_1_network->output->acti_vals[0];
		cout << "branch_1_predicted: " << branch_1_predicted << endl;
		branch_2_network->activate(inputs);
		double branch_2_predicted = branch_2_network->output->acti_vals[0];
		cout << "branch_2_predicted: " << branch_2_predicted << endl;

		cout << endl;
	}

	for (int iter_index = 0; iter_index < 500000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		double input_1 = input_distribution(generator);
		double input_2 = input_distribution(generator);
		vector<double> inputs{input_1, input_2};

		branch_1_network->activate(inputs);
		double branch_1_predicted = branch_1_network->output->acti_vals[0];
		branch_2_network->activate(inputs);
		double branch_2_predicted = branch_2_network->output->acti_vals[0];

		if (branch_1_predicted > branch_2_predicted) {
			double target_val;
			if (input_1 == input_2) {
				target_val = -1.0 + noise_distribution(generator);
			} else {
				target_val = 1.0 + noise_distribution(generator);
			}

			vector<double> errors{target_val - branch_1_predicted};
			branch_1_network->backprop(errors);

			branch_1_epoch++;
			if (branch_1_epoch >= 20) {
				branch_1_network->update();

				branch_1_epoch = 0;
			}
		} else {
			double target_val;
			if (input_1 == input_2) {
				target_val = 1.0 + noise_distribution(generator);
			} else {
				target_val = -1.0 + noise_distribution(generator);
			}

			vector<double> errors{target_val - branch_2_predicted};
			branch_2_network->backprop(errors);

			branch_2_epoch++;
			if (branch_2_epoch >= 20) {
				branch_2_network->update();

				branch_2_epoch = 0;
			}
		}
	}

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		cout << "post " << iter_index << endl;

		double input_1 = input_distribution(generator);
		cout << "input_1: " << input_1 << endl;
		double input_2 = input_distribution(generator);
		cout << "input_2: " << input_2 << endl;
		vector<double> inputs{input_1, input_2};

		branch_1_network->activate(inputs);
		double branch_1_predicted = branch_1_network->output->acti_vals[0];
		cout << "branch_1_predicted: " << branch_1_predicted << endl;
		branch_2_network->activate(inputs);
		double branch_2_predicted = branch_2_network->output->acti_vals[0];
		cout << "branch_2_predicted: " << branch_2_predicted << endl;

		cout << endl;
	}

	cout << "Done" << endl;
}
