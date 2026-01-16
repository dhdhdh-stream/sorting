#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_INPUTS = 10;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	int num_full_better = 0;
	int num_limit_better = 0;

	for (int epoch_index = 0; epoch_index < 1000; epoch_index++) {
		uniform_int_distribution<int> input_distribution(-10, 10);
		uniform_int_distribution<int> noise_distribution(-100, 100);

		vector<vector<double>> train_obs_vals;
		vector<double> train_true_vals;
		for (int i_index = 0; i_index < 1000; i_index++) {
			double val = input_distribution(generator);

			vector<double> input;
			input.push_back(val);
			for (int ii_index = 0; ii_index < NUM_INPUTS-1; ii_index++) {
				input.push_back(input_distribution(generator));
			}

			double noise = noise_distribution(generator);

			train_obs_vals.push_back(input);
			train_true_vals.push_back(val + noise);
		}

		uniform_int_distribution<int> train_distribution(0, train_obs_vals.size()-1);

		Network* full_network = new Network(NUM_INPUTS,
											NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			full_network->activate(train_obs_vals[rand_index]);

			double error = train_true_vals[rand_index] - full_network->output->acti_vals[0];

			full_network->backprop(error);
		}

		Network* limit_network = new Network(1,
											 NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			vector<double> limit_input{train_obs_vals[rand_index][0]};
			limit_network->activate(limit_input);

			double error = train_true_vals[rand_index] - limit_network->output->acti_vals[0];

			limit_network->backprop(error);
		}

		double sum_full_misguess = 0.0;
		double sum_limit_misguess = 0.0;
		for (int i_index = 0; i_index < 1000; i_index++) {
			double val = input_distribution(generator);

			vector<double> input;
			input.push_back(val);
			for (int ii_index = 0; ii_index < NUM_INPUTS-1; ii_index++) {
				input.push_back(input_distribution(generator));
			}

			double noise = noise_distribution(generator);

			double result = val + noise;

			full_network->activate(input);
			double full_predicted = full_network->output->acti_vals[0];
			sum_full_misguess += (full_predicted - result) * (full_predicted - result);

			vector<double> limit_input{input[0]};
			limit_network->activate(limit_input);
			double limit_predicted = limit_network->output->acti_vals[0];
			sum_limit_misguess += (limit_predicted - result) * (limit_predicted - result);
		}

		cout << "sum_full_misguess: " << sum_full_misguess << endl;
		cout << "sum_limit_misguess: " << sum_limit_misguess << endl;

		if (sum_limit_misguess < sum_full_misguess) {
			num_limit_better++;
		} else {
			num_full_better++;
		}

		cout << "num_full_better: " << num_full_better << endl;
		cout << "num_limit_better: " << num_limit_better << endl;

		delete full_network;
		delete limit_network;
	}

	cout << "Done" << endl;
}
