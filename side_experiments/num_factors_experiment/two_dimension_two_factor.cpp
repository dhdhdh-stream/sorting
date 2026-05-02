// TODO: need negative examples
// - add indirect network

// - issue is that multi-dimensional things can be squashed to be on the same dimension
//   - especially without negative

// TODO: try simply magnifiying diff
// - though can let it go if diff too big

// - perhaps not about is 2-dimensional more accurate than 1
//   - unlikely, as if you squash it, unlikely for predictions to be that different
// - but about can you treat something as 2 separate things?

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

	// geometric_distribution<int> num_actions_distribution(0.2);
	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	while (true) {
		int num_actions = num_actions_distribution(generator);
		vector<int> curr_sequence;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
		}

		int sum_distance_one = 0;
		int sum_distance_two = 0;
		for (int a_index = 0; a_index < (int)curr_sequence.size(); a_index++) {
			switch (curr_sequence[a_index]) {
			case 0:
				sum_distance_one--;
				break;
			case 1:
				sum_distance_one++;
				break;
			case 2:
				sum_distance_two--;
				break;
			case 3:
				sum_distance_two++;
				break;
			}
		}

		if (sum_distance_one == 2
				&& sum_distance_two == 1) {
			sequences.push_back(curr_sequence);

			// if (sequences.size() >= 1000) {
			if (sequences.size() >= 4000) {
				break;
			}
		}
	}

	Network* network_one = new Network(4);
	Network* network_two = new Network(4);

	uniform_int_distribution<int> sequence_distribution(0, sequences.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double sum_impact_one = 0.0;
		double sum_impact_two = 0.0;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < 4; i_index++) {
				if (i_index == sequences[sequence_index][a_index]) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			network_one->activate(inputs);
			network_two->activate(inputs);

			sum_impact_one += network_one->output->acti_vals[0];
			sum_impact_two += network_two->output->acti_vals[0];
		}

		double average_error_one = (1.0 - sum_impact_one) / (double)sequences[sequence_index].size();
		double average_error_two = (1.0 - sum_impact_two) / (double)sequences[sequence_index].size();

		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < 4; i_index++) {
				if (i_index == sequences[sequence_index][a_index]) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			network_one->activate(inputs);
			network_two->activate(inputs);

			network_one->backprop(average_error_one);
			network_two->backprop(average_error_two);
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
		network_one->activate(inputs);
		network_two->activate(inputs);

		cout << a_index << endl;
		cout << "one: " << network_one->output->acti_vals[0] << endl;
		cout << "two: " << network_two->output->acti_vals[0] << endl;
	}

	cout << "Done" << endl;
}
