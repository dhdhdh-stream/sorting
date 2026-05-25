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

const int NUM_OBS = 5;
const int SEQUENCE_LENGTH = 10;

const int NUM_SAMPLES = 10000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<vector<double>>> sequences;
	vector<double> target_vals;

	uniform_int_distribution<int> xor_distribution(0, 1);
	uniform_int_distribution<int> starting_distribution(0, 2);

	Network* network = new Network(LEAKY_LAYER, 40, 1);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int val_1;
		if (starting_distribution(generator) == 0) {
			val_1 = 1;
		} else {
			val_1 = 0;
		}
		int val_2 = xor_distribution(generator);

		double target_val;
		if (val_1 == val_2) {
			target_val = 1.0;
		} else {
			target_val = -1.0;
		}

		vector<double> inputs;
		inputs.push_back(val_1);
		inputs.push_back(val_2);
		for (int i_index = 2; i_index < 40; i_index++) {
			inputs.push_back(xor_distribution(generator));
		}

		network->activate(inputs);

		vector<double> errors{target_val - network->output->acti_vals[0]};
		network->backprop(errors);

		if (iter_index % 20 == 0) {
			network->update();
		}
	}

	for (int iter_index = 0; iter_index < 20; iter_index++) {
		cout << iter_index << endl;

		int val_1;
		if (starting_distribution(generator) == 0) {
			val_1 = 1;
		} else {
			val_1 = 0;
		}
		int val_2 = xor_distribution(generator);

		cout << "val_1: " << val_1 << endl;
		cout << "val_2: " << val_2 << endl;

		double target_val;
		if (val_1 == val_2) {
			target_val = 1.0;
		} else {
			target_val = -1.0;
		}

		vector<double> inputs;
		inputs.push_back(val_1);
		inputs.push_back(val_2);
		for (int i_index = 2; i_index < 40; i_index++) {
			inputs.push_back(xor_distribution(generator));
		}

		network->activate(inputs);

		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
		cout << "target_val: " << target_val << endl;
	}

	cout << "Done" << endl;
}
