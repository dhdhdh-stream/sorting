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

	vector<vector<int>> sequences;

	geometric_distribution<int> num_actions_distribution(0.2);
	uniform_int_distribution<int> action_distribution(0, 3);
	while (true) {
		int num_actions = num_actions_distribution(generator);
		vector<int> curr_sequence;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
		}

		int sum_distance = 0;
		for (int a_index = 0; a_index < (int)curr_sequence.size(); a_index++) {
			switch (curr_sequence[a_index]) {
			case 0:
				sum_distance--;
				break;
			case 1:
				sum_distance++;
				break;
			}
		}

		if (sum_distance == 2) {
			sequences.push_back(curr_sequence);

			if (sequences.size() >= 1000) {
				break;
			}
		}
	}

	Network* network = new Network(4);

	uniform_int_distribution<int> sequence_distribution(0, sequences.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double sum_impact = 0.0;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < 4; i_index++) {
				if (i_index == sequences[sequence_index][a_index]) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			network->activate(inputs);

			sum_impact += network->output->acti_vals[0];
		}

		double average_error = (1.0 - sum_impact) / (double)sequences[sequence_index].size();

		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < 4; i_index++) {
				if (i_index == sequences[sequence_index][a_index]) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			network->activate(inputs);

			network->backprop(average_error);
		}
	}

	for (int a_index = 0; a_index < 4; a_index++) {
		vector<double> inputs;
		for (int i_index = 0; i_index < 4; i_index++) {
			if (i_index == a_index) {
				inputs.push_back(1.0);
			} else {
				inputs.push_back(0.0);
			}
		}
		network->activate(inputs);

		cout << a_index << ": " << network->output->acti_vals[0] << endl;
	}

	cout << "Done" << endl;
}
