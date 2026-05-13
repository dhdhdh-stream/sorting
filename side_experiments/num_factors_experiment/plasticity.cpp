/**
 * - leaky way better
 */

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

	// Network* network = new Network(SIGMOID_LAYER, 2, 1);
	Network* network = new Network(LEAKY_LAYER, 2, 1);

	{
		uniform_int_distribution<int> obs_distribution(0, 1);
		for (int iter_index = 0; iter_index < 500000; iter_index++) {
			if (iter_index % 10000 == 0) {
				cout << iter_index << endl;
			}

			double o1 = obs_distribution(generator);
			double o2 = obs_distribution(generator);

			double target_val;
			if (o1 == o2) {
				target_val = 100.0;
			} else {
				target_val = -100.0;
			}

			vector<double> inputs{o1, o2};
			network->activate(inputs);

			vector<double> errors{target_val - network->output->acti_vals[0]};
			network->backprop(errors);

			if ((iter_index+1) % 20 == 0) {
				network->update();
			}
		}

		for (int iter_index = 0; iter_index < 10; iter_index++) {
			double o1 = obs_distribution(generator);
			cout << "o1: " << o1 << endl;
			double o2 = obs_distribution(generator);
			cout << "o2: " << o2 << endl;

			double target_val;
			if (o1 == o2) {
				target_val = 100.0;
			} else {
				target_val = -100.0;
			}
			cout << "target_val: " << target_val << endl;

			vector<double> inputs{o1, o2};
			network->activate(inputs);
			cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
		}

		double sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < 4000; iter_index++) {
			double o1 = obs_distribution(generator);
			double o2 = obs_distribution(generator);

			double target_val;
			if (o1 == o2) {
				target_val = 100.0;
			} else {
				target_val = -100.0;
			}

			vector<double> inputs{o1, o2};
			network->activate(inputs);
			sum_misguess += (target_val - network->output->acti_vals[0])
				* (target_val - network->output->acti_vals[0]);
		}
		double average_misguess = sum_misguess / 4000.0;
		cout << "average_misguess: " << average_misguess << endl;
	}

	{
		uniform_int_distribution<int> obs_distribution(-10, 10);
		for (int iter_index = 0; iter_index < 500000; iter_index++) {
			if (iter_index % 10000 == 0) {
				cout << iter_index << endl;
			}

			double o1 = obs_distribution(generator);
			double o2 = obs_distribution(generator);

			double target_val = o1 + o2;

			vector<double> inputs{o1, o2};
			network->activate(inputs);

			vector<double> errors{target_val - network->output->acti_vals[0]};
			network->backprop(errors);

			if ((iter_index+1) % 20 == 0) {
				network->update();
			}
		}

		for (int iter_index = 0; iter_index < 10; iter_index++) {
			double o1 = obs_distribution(generator);
			cout << "o1: " << o1 << endl;
			double o2 = obs_distribution(generator);
			cout << "o2: " << o2 << endl;

			double target_val = o1 + o2;
			cout << "target_val: " << target_val << endl;

			vector<double> inputs{o1, o2};
			network->activate(inputs);
			cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
		}

		double sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < 4000; iter_index++) {
			double o1 = obs_distribution(generator);
			double o2 = obs_distribution(generator);

			double target_val = o1 + o2;

			vector<double> inputs{o1, o2};
			network->activate(inputs);
			sum_misguess += (target_val - network->output->acti_vals[0])
				* (target_val - network->output->acti_vals[0]);
		}
		double average_misguess = sum_misguess / 4000.0;
		cout << "average_misguess: " << average_misguess << endl;
	}

	{
		uniform_int_distribution<int> obs_distribution(0, 1);
		for (int iter_index = 0; iter_index < 500000; iter_index++) {
			if (iter_index % 10000 == 0) {
				cout << iter_index << endl;
			}

			double o1 = obs_distribution(generator);
			double o2 = obs_distribution(generator);

			double target_val;
			if (o1 == o2) {
				target_val = -1.0;
			} else {
				target_val = 1.0;
			}

			vector<double> inputs{o1, o2};
			network->activate(inputs);

			vector<double> errors{target_val - network->output->acti_vals[0]};
			network->backprop(errors);

			if ((iter_index+1) % 20 == 0) {
				network->update();
			}
		}

		for (int iter_index = 0; iter_index < 10; iter_index++) {
			double o1 = obs_distribution(generator);
			cout << "o1: " << o1 << endl;
			double o2 = obs_distribution(generator);
			cout << "o2: " << o2 << endl;

			double target_val;
			if (o1 == o2) {
				target_val = -1.0;
			} else {
				target_val = 1.0;
			}
			cout << "target_val: " << target_val << endl;

			vector<double> inputs{o1, o2};
			network->activate(inputs);
			cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
		}

		double sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < 4000; iter_index++) {
			double o1 = obs_distribution(generator);
			double o2 = obs_distribution(generator);

			double target_val;
			if (o1 == o2) {
				target_val = -1.0;
			} else {
				target_val = 1.0;
			}

			vector<double> inputs{o1, o2};
			network->activate(inputs);
			sum_misguess += (target_val - network->output->acti_vals[0])
				* (target_val - network->output->acti_vals[0]);
		}
		double average_misguess = sum_misguess / 4000.0;
		cout << "average_misguess: " << average_misguess << endl;
	}

	cout << "Done" << endl;
}
