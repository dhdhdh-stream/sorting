// - not good enough

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"
#include "world_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* network = new Network(25,
								   NETWORK_SIZE_LARGE);

	// for (int iter_index = 0; iter_index < 1000000; iter_index++) {
	for (int iter_index = 0; iter_index < 2000000; iter_index++) {
		vector<double> obs;
		double result;
		get_instance(obs,
					 result);

		network->activate(obs);

		double error = result - network->output->acti_vals[0];

		network->backprop(error);

		if (iter_index % 100000 == 0) {
			cout << iter_index << endl;
		}
	}

	for (int i_index = 0; i_index < 20; i_index++) {
		vector<double> obs;
		double result;
		get_instance(obs,
					 result);

		network->activate(obs);

		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << obs[5 * y_index + x_index] << " ";
			}
			cout << endl;
		}

		cout << "result: " << result << endl;

		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	}

	cout << "Done" << endl;
}
