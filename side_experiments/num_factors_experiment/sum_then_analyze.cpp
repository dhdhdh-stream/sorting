/**
 * - can't really add context if simply summing?
 *   - but can't really get anything at all if double backprop-ing impact?
 * 
 * - try starting at 1, then shifting impact?
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

const int NUM_SAMPLES = 4000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<int>> sequences;
	vector<double> target_vals;

	geometric_distribution<int> num_actions_distribution(0.2);
	uniform_int_distribution<int> action_distribution(0, 3);
	for (int i_index = 0; i_index < NUM_SAMPLES; i_index++) {
		int num_actions = 1 + num_actions_distribution(generator);
		vector<int> curr_sequence;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
		}
		sequences.push_back(curr_sequence);

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
		if (sum_distance >= 1) {
			target_vals.push_back(1.0);
		} else {
			target_vals.push_back(0.0);
		}
	}

	Network* network = new Network(4);

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<double> inputs(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			inputs[sequences[sequence_index][a_index]] += 1.0;
		}
		network->activate(inputs);

		double error = target_vals[sequence_index] - network->output->acti_vals[0];
		network->backprop(error);
	}

	for (int h_index = 0; h_index < 10; h_index++) {
		cout << h_index << endl;

		vector<double> inputs(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			inputs[sequences[h_index][a_index]] += 1.0;
		}
		network->activate(inputs);

		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;
	}

	cout << "Done" << endl;
}
