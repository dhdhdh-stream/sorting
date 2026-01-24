#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_TRAIN_DATA = 1000;
const int NUM_INPUTS = 20;

const int NUM_DATA_PER_EPOCH = 100;

// const int MAX_WEIGHT = 19;
const int MAX_WEIGHT = 9;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	uniform_int_distribution<int> input_distribution(-1, 1);
	uniform_int_distribution<int> noise_distribution(-10, 10);

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

	// for (int init_index = 0; init_index < 10; init_index++) {
	for (int init_index = 0; init_index < 1; init_index++) {
		for (int t_index = 0; t_index < NUM_TRAIN_DATA; t_index++) {
			network->activate(train_data[t_index]);

			double error = train_targets[t_index] - network->output->acti_vals[0];

			network->backprop(error);
		}
	}

	int data_index = 0;
	int long_iter = 1;

	// for (int epoch_index = 0; epoch_index < 100; epoch_index++) {
	for (int epoch_index = 0; epoch_index < 200; epoch_index++) {
		for (int d_index = 0; d_index < NUM_DATA_PER_EPOCH; d_index++) {
			vector<double> inputs(NUM_INPUTS);
			for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
				inputs[i_index] = input_distribution(generator);
			}
			double noise = noise_distribution(generator);

			double result = inputs[0] + noise;

			train_data[data_index] = inputs;
			train_targets[data_index] = result;

			data_index++;
			if (data_index >= NUM_TRAIN_DATA) {
				data_index = 0;
				long_iter++;

				// temp
				{
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
				}
			}
		}

		vector<double> existing_vals(NUM_TRAIN_DATA);
		for (int t_index = 0; t_index < NUM_TRAIN_DATA; t_index++) {
			network->activate(train_data[t_index]);
			existing_vals[t_index] = network->output->acti_vals[0];
		}

		vector<double> adjusted_targets(NUM_TRAIN_DATA);
		for (int t_index = 0; t_index < NUM_TRAIN_DATA; t_index++) {
			if (long_iter >= MAX_WEIGHT) {
				adjusted_targets[t_index] = (MAX_WEIGHT * existing_vals[t_index] + train_targets[t_index]) / (1.0 + MAX_WEIGHT);
			} else {
				adjusted_targets[t_index] = (long_iter * existing_vals[t_index] + train_targets[t_index]) / (1.0 + long_iter);
			}
		}

		for (int t_index = 0; t_index < 1000; t_index++) {
			network->activate(train_data[t_index]);

			double error = adjusted_targets[t_index] - network->output->acti_vals[0];

			network->backprop(error);
		}
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		vector<double> inputs(NUM_INPUTS);
		for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
			inputs[i_index] = input_distribution(generator);
		}

		network->activate(inputs);

		cout << "inputs[0]: " << inputs[0] << endl;
		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
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

	cout << "Done" << endl;
}
