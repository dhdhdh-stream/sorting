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

const double MIN_RATIO = 0.0001;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<Network*> networks;
	networks.push_back(new Network(1));
	networks.push_back(new Network(1));

	Network* final_network = new Network(2);

	uniform_int_distribution<int> input_distribution(0, 1);
	// uniform_int_distribution<int> input_distribution(0, 3);
	// for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
	for (int iter_index = 0; iter_index < 1000000; iter_index++) {
		int input = input_distribution(generator);
		// int input;
		// if (input_distribution(generator) == 0) {
		// 	input = 1;
		// } else {
		// 	input = 0;
		// }

		vector<double> predicted(networks.size());
		vector<double> inputs{(double)input};
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			networks[n_index]->activate(inputs);
			predicted[n_index] = networks[n_index]->output->acti_vals[0];
			if (predicted[n_index] < 0.0) {
				predicted[n_index] = 0.0;
			}
		}

		double sum_predicted = 0.0;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			sum_predicted += predicted[n_index];
		}
		if (sum_predicted < MIN_RATIO) {
			sum_predicted = MIN_RATIO;
		}

		vector<double> ratios(networks.size());
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			ratios[n_index] = 1.0 - predicted[n_index] / sum_predicted;
			if (ratios[n_index] < MIN_RATIO) {
				ratios[n_index] = MIN_RATIO;
			}
		}

		double sum_ratios = 0.0;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			sum_ratios += ratios[n_index];
		}

		uniform_real_distribution<double> distribution(0.0, sum_ratios);
		double rand_val = distribution(generator);
		int selected_index;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			rand_val -= ratios[n_index];
			if (rand_val <= 0.0) {
				selected_index = n_index;
				break;
			}
		}

		vector<double> final_inputs(networks.size());
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			if (n_index == selected_index) {
				final_inputs[n_index] = 1.0;
			} else {
				final_inputs[n_index] = 0.0;
			}
		}

		final_network->activate(final_inputs);

		double target_val;
		if (input == 0) {
			target_val = 1.0;
		} else {
			// target_val = -1.0;
			target_val = 0.0;
		}
		double final_error = target_val - final_network->output->acti_vals[0];
		final_network->backprop(final_error);

		double target_error = 0.0;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			target_error += abs(final_network->input->errors[n_index]);
			final_network->input->errors[n_index] = 0.0;
		}

		double error = target_error - networks[selected_index]->output->acti_vals[0];
		networks[selected_index]->backprop(error);

		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
			cout << "input: " << input << endl;
			cout << "predicted[0]: " << predicted[0] << endl;
			cout << "predicted[1]: " << predicted[1] << endl;
			cout << "ratios[0]: " << ratios[0] << endl;
			cout << "ratios[1]: " << ratios[1] << endl;
			cout << "selected_index: " << selected_index << endl;
			cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;
			cout << "target_error: " << target_error << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
