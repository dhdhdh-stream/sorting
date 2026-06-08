// - definitely need to center networks
// - perhaps just randomly assign val
// - or maybe only reliable way is to split

// - maybe can have swap:
//   - split one network while removing another

#include <algorithm>
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

const int NUM_NETWORKS = 2;

const int MEASURE_ITERS = 4000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<Network*> val_networks;
	for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
		val_networks.push_back(new Network(1, 1));
	}

	vector<Network*> select_networks;
	for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
		select_networks.push_back(new Network(1, 1));
	}

	uniform_int_distribution<int> input_distribution(0, 1);
	uniform_int_distribution<int> target_1_distribution(0, 3);
	uniform_int_distribution<int> target_2_distribution(0, 1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int input = input_distribution(generator);

		double target_val;
		switch (input) {
		case 0:
			if (target_1_distribution(generator) == 0) {
				target_val = 2.0;
			} else {
				target_val = -0.5;
			}
			break;
		case 1:
			if (target_2_distribution(generator) == 0) {
				target_val = 1.0;
			} else {
				target_val = -1.0;
			}
			break;
		}

		vector<double> inputs{(double)input};

		int min_index;
		double min_error = numeric_limits<double>::max();
		vector<double> predicted(NUM_NETWORKS);
		for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
			val_networks[n_index]->activate(inputs);
			predicted[n_index] = val_networks[n_index]->output->acti_vals[0];
			double error = (target_val - val_networks[n_index]->output->acti_vals[0])
				* (target_val - val_networks[n_index]->output->acti_vals[0]);
			if (error < min_error) {
				min_index = n_index;
				min_error = error;
			}
		}

		{
			vector<double> errors{target_val - val_networks[min_index]->output->acti_vals[0]};
			val_networks[min_index]->backprop(errors);
		}

		vector<double> select_vals(NUM_NETWORKS);
		for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
			select_networks[n_index]->activate(inputs);
			select_vals[n_index] = select_networks[n_index]->output->acti_vals[0];
		}

		double max_select_val = select_vals[0];
		for (int n_index = 1; n_index < NUM_NETWORKS; n_index++) {
			if (select_vals[n_index] > max_select_val) {
				max_select_val = select_vals[n_index];
			}
		}
		for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
			select_vals[n_index] -= max_select_val;
		}

		for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
			// select_vals[n_index] = exp(0.1 * select_vals[n_index]);
			select_vals[n_index] = exp(select_vals[n_index]);
		}

		double sum_select = 0.0;
		for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
			sum_select += select_vals[n_index];
		}
		for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
			select_vals[n_index] /= sum_select;
		}

		for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
			if (n_index == min_index) {
				vector<double> errors{1.0 - select_vals[n_index]};
				select_networks[n_index]->backprop(errors);
			} else {
				vector<double> errors{-select_vals[n_index]};
				select_networks[n_index]->backprop(errors);
			}
		}

		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
			cout << "input: " << input << endl;
			cout << "target_val: " << target_val << endl;
			cout << "predicted:" << endl;
			for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
				cout << n_index << ": " << predicted[n_index] << endl;
			}
			cout << "select_vals:" << endl;
			for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
				cout << n_index << ": " << select_vals[n_index] << endl;
			}
			cout << "select_networks[n_index]->output->acti_vals[0]:" << endl;
			for (int n_index = 0; n_index < NUM_NETWORKS; n_index++) {
				cout << n_index << ": " << select_networks[n_index]->output->acti_vals[0] << endl;
			}
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
