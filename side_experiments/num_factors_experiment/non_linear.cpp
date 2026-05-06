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
	Network* final_network = new Network(1);

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);
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

		vector<double> final_inputs{sum_impact};
		final_network->activate(final_inputs);

		double final_error = target_vals[sequence_index] - final_network->output->acti_vals[0];
		final_network->backprop(final_error);

		double error = final_network->input->errors[0];
		// temp
		if (iter_index % 10000 == 0) {
			cout << "error: " << error << endl;
		}
		final_network->input->errors[0] = 0.0;
		double average_error = error / (double)sequences[sequence_index].size();

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

	for (int h_index = 0; h_index < 10; h_index++) {
		cout << h_index << endl;

		double sum_impact = 0.0;
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < 4; i_index++) {
				if (i_index == sequences[h_index][a_index]) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			network->activate(inputs);

			sum_impact += network->output->acti_vals[0];
		}

		cout << "sum_impact: " << sum_impact << endl;

		vector<double> final_inputs{sum_impact};
		final_network->activate(final_inputs);

		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;
	}

	cout << "Done" << endl;
}
