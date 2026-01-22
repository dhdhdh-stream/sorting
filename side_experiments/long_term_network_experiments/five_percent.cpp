#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_TRAIN_DATA = 20000;
const int NUM_INPUTS = 20;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	uniform_int_distribution<int> input_distribution(-1, 1);
	uniform_int_distribution<int> noise_distribution(-20, 20);

	uniform_int_distribution<int> train_distribution(0, NUM_TRAIN_DATA-1);

	for (int try_index = 0; try_index < 10; try_index++) {
		vector<vector<double>> train_data(NUM_TRAIN_DATA);
		vector<double> train_targets(NUM_TRAIN_DATA);
		for (int t_index = 0; t_index < NUM_TRAIN_DATA; t_index++) {
			vector<double> inputs(NUM_INPUTS);
			for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
				inputs[i_index] = input_distribution(generator);
			}
			double noise = noise_distribution(generator);

			double result = inputs[0] + noise;

			train_data[t_index] = inputs;
			train_targets[t_index] = result;
		}

		Network* network = new Network(NUM_INPUTS);

		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			network->activate(train_data[rand_index]);

			double error = train_targets[rand_index] - network->output->acti_vals[0];

			network->backprop(error);

			if (iter_index % 100000 == 0) {
				cout << iter_index << endl;
			}
		}

		double sum_misguess = 0.0;
		for (int i_index = 0; i_index < 1000; i_index++) {
			vector<double> inputs(NUM_INPUTS);
			for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
				inputs[i_index] = input_distribution(generator);
			}

			network->activate(inputs);

			sum_misguess += (inputs[0] - network->output->acti_vals[0]) * (inputs[0] - network->output->acti_vals[0]);
		}
		cout << "sum_misguess: " << sum_misguess << endl;

		delete network;
	}

	cout << "Done" << endl;
}
