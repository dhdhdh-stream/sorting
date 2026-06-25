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

	Network* noise_network = new Network(1);
	double hidden_1_average_max_update = 0.0;
	double hidden_2_average_max_update = 0.0;
	double hidden_3_average_max_update = 0.0;
	double output_average_max_update = 0.0;
	Network* error_network = new Network(1);
	double error_hidden_1_average_max_update = 0.0;
	double error_hidden_2_average_max_update = 0.0;
	double error_hidden_3_average_max_update = 0.0;
	double error_output_average_max_update = 0.0;
	Network* ed_network = new Network(1);
	double ed_hidden_1_average_max_update = 0.0;
	double ed_hidden_2_average_max_update = 0.0;
	double ed_hidden_3_average_max_update = 0.0;
	double ed_output_average_max_update = 0.0;

	uniform_int_distribution<int> base_distribution(0, 1);
	normal_distribution<double> noise_distribution(0, 1);
	for (int iter_index = 0; iter_index < 10000000; iter_index++) {
		double base = -0.5 + base_distribution(generator);
		double noise = noise_distribution(generator);

		vector<double> inputs{base + noise};
		noise_network->activate(inputs);

		double error = noise - noise_network->output->acti_vals[0];

		noise_network->init_backprop(error,
									 hidden_1_average_max_update,
									 hidden_2_average_max_update,
									 hidden_3_average_max_update,
									 output_average_max_update);

		error_network->activate(inputs);

		double error_error = abs(error) - error_network->output->acti_vals[0];

		error_network->init_backprop(error_error,
									 error_hidden_1_average_max_update,
									 error_hidden_2_average_max_update,
									 error_hidden_3_average_max_update,
									 error_output_average_max_update);

		ed_network->activate(inputs);

		double ed_error;
		if (noise > noise_network->output->acti_vals[0]) {
			if (ed_network->output->acti_vals[0] > 1.0) {
				ed_error = 0.0;
			} else {
				ed_error = 1.0 - ed_network->output->acti_vals[0];
			}
		} else {
			if (ed_network->output->acti_vals[0] < -1.0) {
				ed_error = 0.0;
			} else {
				ed_error = -1.0 - ed_network->output->acti_vals[0];
			}
		}

		ed_network->init_backprop(ed_error,
								  ed_hidden_1_average_max_update,
								  ed_hidden_2_average_max_update,
								  ed_hidden_3_average_max_update,
								  ed_output_average_max_update);

		double adjusted = base + noise - noise_network->output->acti_vals[0];

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;
			cout << "base: " << base << endl;
			cout << "noise: " << noise << endl;
			cout << "noise_network->output->acti_vals[0]: " << noise_network->output->acti_vals[0] << endl;
			cout << "adjusted: " << adjusted << endl;
			cout << "error_network->output->acti_vals[0]: " << error_network->output->acti_vals[0] << endl;
			cout << "ed_error: " << ed_error << endl;
			cout << "ed_network->output->acti_vals[0]: " << ed_network->output->acti_vals[0] << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
