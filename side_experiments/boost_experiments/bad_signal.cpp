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

	int num_direct_better = 0;
	int num_indirect_better = 0;

	for (int epoch_index = 0; epoch_index < 1000; epoch_index++) {
		uniform_int_distribution<int> input_distribution(-10, 10);
		uniform_int_distribution<int> noise_distribution(-100, 100);

		vector<vector<double>> train_obs_vals;
		vector<vector<double>> train_signal_vals;
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
			train_signal_vals.push_back(input);
			train_true_vals.push_back(val + noise);
		}

		uniform_int_distribution<int> train_distribution(0, train_obs_vals.size()-1);

		Network* signal_network = new Network(NUM_INPUTS,
											  NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			signal_network->activate(train_signal_vals[rand_index]);

			double error = train_true_vals[rand_index] - signal_network->output->acti_vals[0];

			signal_network->backprop(error);
		}

		vector<double> train_tunnel_vals(train_obs_vals.size());
		for (int h_index = 0; h_index < (int)train_obs_vals.size(); h_index++) {
			signal_network->activate(train_signal_vals[h_index]);
			train_tunnel_vals[h_index] = signal_network->output->acti_vals[0];
		}

		Network* direct_network = new Network(NUM_INPUTS,
											  NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			direct_network->activate(train_obs_vals[rand_index]);

			double error = train_true_vals[rand_index] - direct_network->output->acti_vals[0];

			direct_network->backprop(error);
		}

		Network* indirect_network = new Network(NUM_INPUTS,
												NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int rand_index = train_distribution(generator);

			indirect_network->activate(train_obs_vals[rand_index]);

			double error = train_tunnel_vals[rand_index] - indirect_network->output->acti_vals[0];

			indirect_network->backprop(error);
		}

		double sum_direct_misguess = 0.0;
		double sum_indirect_misguess = 0.0;
		for (int i_index = 0; i_index < 1000; i_index++) {
			double val = input_distribution(generator);

			vector<double> input;
			input.push_back(val);
			for (int ii_index = 0; ii_index < NUM_INPUTS-1; ii_index++) {
				input.push_back(input_distribution(generator));
			}

			double noise = noise_distribution(generator);

			double result = val + noise;

			direct_network->activate(input);
			double direct_predicted = direct_network->output->acti_vals[0];
			sum_direct_misguess += (direct_predicted - result) * (direct_predicted - result);

			indirect_network->activate(input);
			double indirect_predicted = indirect_network->output->acti_vals[0];
			sum_indirect_misguess += (indirect_predicted - result) * (indirect_predicted - result);
		}

		cout << "sum_direct_misguess: " << sum_direct_misguess << endl;
		cout << "sum_indirect_misguess: " << sum_indirect_misguess << endl;

		if (sum_indirect_misguess < sum_direct_misguess) {
			num_indirect_better++;
		} else {
			num_direct_better++;
		}

		cout << "num_direct_better: " << num_direct_better << endl;
		cout << "num_indirect_better: " << num_indirect_better << endl;

		delete signal_network;
		delete direct_network;
		delete indirect_network;
	}

	cout << "Done" << endl;
}
