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

	Network* network_1 = new Network(LEAKY_LAYER, 2, 1);

	uniform_int_distribution<int> val_distribution(0, 1);
	uniform_int_distribution<int> special_case_distribution(0, 4);
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		double val_1 = val_distribution(generator);
		double val_2 = val_distribution(generator);

		vector<double> inputs{val_1, val_2};
		network_1->activate(inputs);

		double target_val;
		if (special_case_distribution(generator) == 0) {
			if (val_distribution(generator) == 0) {
				target_val = 1.0;
			} else {
				target_val = -1.0;
			}
		} else {
			if (val_1 == val_2) {
				target_val = 1.0;
			} else {
				target_val = -1.0;
			}
		}

		vector<double> errors{target_val - network_1->output->acti_vals[0]};
		network_1->backprop(errors);

		if (iter_index % 20 == 0) {
			network_1->update();
		}
	}

	for (int iter_index = 0; iter_index < 20; iter_index++) {
		cout << iter_index << endl;

		double val_1 = val_distribution(generator);
		cout << "val_1: " << val_1 << endl;
		double val_2 = val_distribution(generator);
		cout << "val_2: " << val_2 << endl;

		vector<double> inputs{val_1, val_2};
		network_1->activate(inputs);

		cout << "network_1->output->acti_vals[0]: " << network_1->output->acti_vals[0] << endl;
	}

	Network* network_2 = new Network(LEAKY_LAYER, 2, 1);
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		double val_1 = val_distribution(generator);
		double val_2 = val_distribution(generator);

		vector<double> inputs{val_1, val_2};
		network_1->activate(inputs);

		bool is_special = special_case_distribution(generator) == 0;
		int special_val = val_distribution(generator);
		if (is_special) {
			vector<double> inputs{1.0, (double)special_val};
			network_2->activate(inputs);
		} else {
			vector<double> inputs{0.0, (double)special_val};
			network_2->activate(inputs);
		}

		double target_val;
		if (is_special) {
			if (special_val == 0) {
				target_val = 1.0;
			} else {
				target_val = -1.0;
			}
		} else {
			if (val_1 == val_2) {
				target_val = 1.0;
			} else {
				target_val = -1.0;
			}
		}

		double predicted = network_1->output->acti_vals[0] + network_2->output->acti_vals[0];

		vector<double> errors{target_val - predicted};
		network_2->backprop(errors);

		if (iter_index % 20 == 0) {
			network_2->update();
		}
	}

	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		double val_1 = val_distribution(generator);
		cout << "val_1: " << val_1 << endl;
		double val_2 = val_distribution(generator);
		cout << "val_2: " << val_2 << endl;

		vector<double> inputs{val_1, val_2};
		network_1->activate(inputs);

		bool is_special = special_case_distribution(generator) == 0;
		cout << "is_special: " << is_special << endl;
		int special_val = val_distribution(generator);
		cout << "special_val: " << special_val << endl;
		if (is_special) {
			vector<double> inputs{1.0, (double)special_val};
			network_2->activate(inputs);
		} else {
			vector<double> inputs{0.0, (double)special_val};
			network_2->activate(inputs);
		}

		cout << "network_1->output->acti_vals[0]: " << network_1->output->acti_vals[0] << endl;
		cout << "network_2->output->acti_vals[0]: " << network_2->output->acti_vals[0] << endl;
	}

	cout << "Done" << endl;
}
