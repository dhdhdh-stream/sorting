#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

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

	Network* mean_network = new Network(2);
	Network* decoder_network = new Network(2);

	uniform_int_distribution<int> input_distribution(0, 1);
	uniform_int_distribution<int> type_1_distribution(0, 1);
	uniform_int_distribution<int> type_2_distribution(0, 3);
	for (int iter_index = 0; iter_index < 10000000; iter_index++) {
		int val = input_distribution(generator);

		double target_val;
		if (val == 0) {
			if (type_1_distribution(generator) == 0) {
				target_val = 2.0;
			} else {
				target_val = 0.5;
			}
		} else {
			if (type_2_distribution(generator) == 0) {
				target_val = 0.2;
			} else {
				target_val = -0.2;
			}
		}

		vector<double> mean_inputs{(double)val, target_val};
		mean_network->activate(mean_inputs);
		double predicted_mean = mean_network->output->acti_vals[0];

		vector<double> decoder_inputs{(double)val, predicted_mean};
		decoder_network->activate(decoder_inputs);
		double predicted_target_val = decoder_network->output->acti_vals[0];

		double decoder_error = target_val - predicted_target_val;
		decoder_network->backprop(decoder_error);

		double mean_error = decoder_network->input->errors[1];
		decoder_network->input->errors[1] = 0.0;

		mean_error -= predicted_mean;

		mean_network->backprop(mean_error);

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;
			cout << "val: " << val << endl;
			cout << "target_val: " << target_val << endl;
			cout << "predicted_mean: " << predicted_mean << endl;
			cout << "predicted_target_val: " << predicted_target_val << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
