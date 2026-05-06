// - watch out for overshoot
//   - lots of multipliers

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
	vector<vector<vector<double>>> contexts;
	vector<double> target_vals;

	geometric_distribution<int> num_actions_distribution(0.2);
	uniform_int_distribution<int> action_distribution(0, 3);
	uniform_int_distribution<int> context_distribution(0, 9);
	for (int i_index = 0; i_index < NUM_SAMPLES; i_index++) {
		int num_actions = 1 + num_actions_distribution(generator);
		
		vector<int> curr_sequence;
		vector<vector<double>> curr_contexts;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
			if (context_distribution(generator) == 0) {
				curr_contexts.push_back(vector<double>{1.0});
			} else {
				curr_contexts.push_back(vector<double>{0.0});
			}
		}
		sequences.push_back(curr_sequence);
		contexts.push_back(curr_contexts);

		int sum_distance = 0;
		for (int a_index = 0; a_index < (int)curr_sequence.size(); a_index++) {
			switch (curr_sequence[a_index]) {
			case 0:
				if (curr_contexts[a_index][0] == 1.0) {
					sum_distance++;
				} else {
					sum_distance--;
				}
				break;
			case 1:
				if (curr_contexts[a_index][0] == 1.0) {
					sum_distance--;
				} else {
					sum_distance++;
				}
				break;
			}
		}
		if (sum_distance >= 1) {
			target_vals.push_back(1.0);
		} else {
			target_vals.push_back(0.0);
		}
	}

	vector<Network*> context_networks;
	for (int a_index = 0; a_index < 4; a_index++) {
		context_networks.push_back(new Network(1));
	}
	Network* network = new Network(4);

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<double> inputs(4, 0.0);
		vector<int> counts(4, 0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			int action = sequences[sequence_index][a_index];

			inputs[action] += 1.0;
			counts[action] += 1;

			Network* context_network = context_networks[action];
			context_network->activate(contexts[sequence_index][a_index]);
			inputs[action] += context_network->output->acti_vals[0];
		}
		network->activate(inputs);

		double error = target_vals[sequence_index] - network->output->acti_vals[0];
		network->backprop(error);

		vector<double> errors(4);
		for (int i_index = 0; i_index < 4; i_index++) {
			errors[i_index] = network->input->errors[i_index];
			network->input->errors[i_index] = 0.0;

			if (counts[i_index] != 0) {
				errors[i_index] /= counts[i_index];
			}
		}

		/**
		 * - need main to settle before adding context
		 */
		if (iter_index > 100000) {
			for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
				int action = sequences[sequence_index][a_index];

				Network* context_network = context_networks[action];
				context_network->activate(contexts[sequence_index][a_index]);

				context_network->backprop(errors[action]);
			}
		}

		// temp
		if (iter_index % 20 == 0) {
			if (iter_index > 100000) {
				for (int a_index = 0; a_index < 4; a_index++) {
					context_networks[a_index]->update();
				}
			}
			network->update();
		}
	}

	for (int h_index = 0; h_index < 20; h_index++) {
		cout << h_index << endl;

		vector<double> inputs(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			int action = sequences[h_index][a_index];
			cout << "action: " << action << endl;

			inputs[action] += 1.0;

			Network* context_network = context_networks[action];
			cout << "context: " << contexts[h_index][a_index][0] << endl;
			context_network->activate(contexts[h_index][a_index]);
			inputs[action] += context_network->output->acti_vals[0];

			cout << "context_network->output->acti_vals[0]: " << context_network->output->acti_vals[0] << endl;
		}
		network->activate(inputs);

		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	cout << "Done" << endl;
}
