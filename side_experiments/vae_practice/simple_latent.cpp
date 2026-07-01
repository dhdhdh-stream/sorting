// - good chance for latent to be correlated anyways
//   - so then no point

// - and learned state should be relatively uncorrelated

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

	Network* encoder = new Network(2, 2);
	Network* decoder = new Network(3, 2);

	uniform_int_distribution<int> type_distribution(0, 1);
	uniform_int_distribution<int> base_distribution(0, 1);
	for (int iter_index = 0; iter_index < 1000000; iter_index++) {
		int type = type_distribution(generator);

		double base_0;
		double base_1;
		if (type == 0) {
			if (base_distribution(generator) == 0) {
				base_0 = 1.0;
				base_1 = -1.0;
			} else {
				base_0 = -1.0;
				base_1 = 1.0;
			}
		} else {
			if (base_distribution(generator) == 0) {
				base_0 = 2.0;
				base_1 = 2.0;
			} else {
				base_0 = -0.3;
				base_1 = 0.3;
			}
		}

		vector<double> encoder_inputs{base_0, base_1};
		encoder->activate(encoder_inputs);

		vector<double> decoder_inputs{encoder->output->acti_vals[0], encoder->output->acti_vals[0], (double)type};
		decoder->activate(decoder_inputs);

		vector<double> decoder_errors{
			base_0 - decoder->output->acti_vals[0],
			base_1 - decoder->output->acti_vals[1]
		};
		decoder->backprop(decoder_errors);

		vector<double> encoder_errors{
			decoder->input->errors[0],
			decoder->input->errors[1],
		};
		decoder->input->errors[0] = 0.0;
		decoder->input->errors[1] = 0.0;

		encoder->backprop(encoder_errors);

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;
			cout << "type: " << type << endl;
			cout << "base_0: " << base_0 << endl;
			cout << "base_1: " << base_1 << endl;
			cout << "encoder->output->acti_vals[0]: " << encoder->output->acti_vals[0] << endl;
			cout << "encoder->output->acti_vals[1]: " << encoder->output->acti_vals[1] << endl;
			cout << "decoder->output->acti_vals[0]: " << decoder->output->acti_vals[0] << endl;
			cout << "decoder->output->acti_vals[1]: " << decoder->output->acti_vals[1] << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
