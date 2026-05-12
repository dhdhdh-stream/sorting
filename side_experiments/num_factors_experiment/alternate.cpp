/***
 * - even with alternate, still need to stabilize first
 * 
 * - actually just better to not alternate
 *   - if actions context dependent, then training without context not going to be sharp
 *     - limits result
 * - can probably still alternate between with action network and with action network + context network
 *   - won't be sharp, but return needs to be calculated with only action network
 * 
 * - TODO: experiment with adding factors
 * - TODO: experiment with low samples
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
	vector<vector<vector<double>>> contexts;
	vector<double> target_vals;

	geometric_distribution<int> num_actions_distribution(0.2);
	uniform_int_distribution<int> action_distribution(0, 3);
	// uniform_int_distribution<int> context_distribution(0, 9);
	uniform_int_distribution<int> context_distribution(0, 4);
	for (int i_index = 0; i_index < NUM_SAMPLES; i_index++) {
		int num_actions = 1 + num_actions_distribution(generator);
		
		vector<int> curr_sequence;
		vector<vector<double>> curr_contexts;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
			if (context_distribution(generator) == 0) {
			// if (false) {
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
		// if (sum_distance >= 1) {
		if (sum_distance == 1) {
			target_vals.push_back(1.0);
		} else {
			target_vals.push_back(0.0);
		}
	}

	// Network* context_network = new Network(SIGMOID_LAYER, 9, 4);
	Network* context_network = new Network(LEAKY_LAYER, 9, 4);
	// Network* network = new Network(SIGMOID_LAYER, 4, 1);
	Network* network = new Network(LEAKY_LAYER, 4, 1);

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);
	uniform_int_distribution<int> type_distribution(0, 1);
	// for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
	for (int iter_index = 0; iter_index < 500000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		int type;
		if (iter_index < 150000) {
			type = 0;
		} else {
			// type = type_distribution(generator);
			type = 1;
		}
		switch (type) {
		case 0:
			{
				vector<double> state(4, 0.0);
				for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
					int action = sequences[sequence_index][a_index];

					state[action] += 1.0;
				}
				network->activate(state);

				vector<double> errors{target_vals[sequence_index] - network->output->acti_vals[0]};
				network->backprop(errors);

				for (int i_index = 0; i_index < 4; i_index++) {
					network->input->errors[i_index] = 0.0;
				}
			}
			break;
		case 1:
			{
				{
					vector<double> state(4, 0.0);
					for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
						int action = sequences[sequence_index][a_index];

						state[action] += 1.0;

						vector<double> inputs = state;
						for (int s_index = 0; s_index < 4; s_index++) {
							if (action == s_index) {
								inputs.push_back(1.0);
							} else {
								inputs.push_back(0.0);
							}
						}
						inputs.push_back(contexts[sequence_index][a_index][0]);
						context_network->activate(inputs);
						for (int s_index = 0; s_index < 4; s_index++) {
							state[s_index] += context_network->output->acti_vals[s_index];
						}
					}
					network->activate(state);
				}

				vector<double> final_errors{target_vals[sequence_index] - network->output->acti_vals[0]};
				network->backprop(final_errors);

				vector<double> errors(4);
				for (int i_index = 0; i_index < 4; i_index++) {
					errors[i_index] = network->input->errors[i_index]
						/ (double)sequences[sequence_index].size();
					network->input->errors[i_index] = 0.0;
				}

				{
					vector<double> state(4, 0.0);
					for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
						int action = sequences[sequence_index][a_index];

						state[action] += 1.0;

						vector<double> inputs = state;
						for (int s_index = 0; s_index < 4; s_index++) {
							if (action == s_index) {
								inputs.push_back(1.0);
							} else {
								inputs.push_back(0.0);
							}
						}
						inputs.push_back(contexts[sequence_index][a_index][0]);
						context_network->activate(inputs);
						for (int s_index = 0; s_index < 4; s_index++) {
							state[s_index] += context_network->output->acti_vals[s_index];
						}
						context_network->backprop(errors);
					}
				}
			}
			break;
		}

		if (iter_index % 20 == 0) {
			if (iter_index >= 150000) {
				context_network->update();
			}
			network->update();
		}
	}

	for (int h_index = 0; h_index < 40; h_index++) {
		cout << h_index << endl;

		vector<double> state(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			int action = sequences[h_index][a_index];
			cout << "action: " << action << endl;

			state[action] += 1.0;

			vector<double> inputs = state;
			for (int s_index = 0; s_index < 4; s_index++) {
				if (action == s_index) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			inputs.push_back(contexts[h_index][a_index][0]);
			cout << "context: " << contexts[h_index][a_index][0] << endl;
			context_network->activate(inputs);
			for (int s_index = 0; s_index < 4; s_index++) {
				state[s_index] += context_network->output->acti_vals[s_index];
				cout << s_index << ": " << state[s_index] << endl;
			}
		}
		network->activate(state);

		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	cout << "Done" << endl;
}
