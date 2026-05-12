/**
 * - actually very good on low samples?
 * 
 * - perhaps state can't depend on state
 *   - otherwise explodes too easily
 * 
 * - can try not phasing out, but also no dependency on state
 *   - will be extremely expensive at runtime though
 * 
 * - or for context networks, simply have no dependency on state
 *   - or at least, no dependency on itself
 * 
 * TODO: also leave some time cleanly training at end
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

const int TRAIN_NUM_SAMPLES = 100;
const int TEST_NUM_SAMPLES = 4000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);

	vector<vector<int>> train_sequences;
	vector<double> train_target_vals;
	for (int i_index = 0; i_index < TRAIN_NUM_SAMPLES; i_index++) {
		int num_actions = 1 + num_actions_distribution(generator);
		
		vector<int> curr_sequence;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
		}
		train_sequences.push_back(curr_sequence);

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

		if (sum_distance == 1) {
			train_target_vals.push_back(1.0);
		} else {
			train_target_vals.push_back(0.0);
		}
	}

	vector<vector<int>> test_sequences;
	vector<double> test_target_vals;
	for (int i_index = 0; i_index < TEST_NUM_SAMPLES; i_index++) {
		int num_actions = 1 + num_actions_distribution(generator);
		
		vector<int> curr_sequence;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
		}
		test_sequences.push_back(curr_sequence);

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

		if (sum_distance == 1) {
			test_target_vals.push_back(1.0);
		} else {
			test_target_vals.push_back(0.0);
		}
	}

	// Network* context_network = new Network(SIGMOID_LAYER, 8, 4);
	Network* context_network = new Network(LEAKY_LAYER, 8, 4);
	// Network* network = new Network(SIGMOID_LAYER, 4, 1);
	Network* network = new Network(LEAKY_LAYER, 4, 1);

	uniform_int_distribution<int> sequence_distribution(0, TRAIN_NUM_SAMPLES-1);
	// for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
	for (int iter_index = 0; iter_index < 500000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		int type;
		if (iter_index < 100000) {
			type = 0;
		} else {
			type = 1;
		}
		switch (type) {
		case 0:
			{
				vector<double> state(4, 0.0);
				for (int a_index = 0; a_index < (int)train_sequences[sequence_index].size(); a_index++) {
					int action = train_sequences[sequence_index][a_index];

					state[action] += 1.0;
				}
				network->activate(state);

				vector<double> errors{train_target_vals[sequence_index] - network->output->acti_vals[0]};
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
					for (int a_index = 0; a_index < (int)train_sequences[sequence_index].size(); a_index++) {
						int action = train_sequences[sequence_index][a_index];

						double ratio = 1.0 - (iter_index - 100000.0) / 400000.0;
						state[action] += ratio;

						vector<double> inputs = state;
						for (int s_index = 0; s_index < 4; s_index++) {
							if (action == s_index) {
								inputs.push_back(1.0);
							} else {
								inputs.push_back(0.0);
							}
						}
						context_network->activate(inputs);
						for (int s_index = 0; s_index < 4; s_index++) {
							state[s_index] += context_network->output->acti_vals[s_index];
						}
					}
					network->activate(state);
				}

				vector<double> final_errors{train_target_vals[sequence_index] - network->output->acti_vals[0]};
				network->backprop(final_errors);

				vector<double> errors(4);
				for (int i_index = 0; i_index < 4; i_index++) {
					errors[i_index] = network->input->errors[i_index]
						/ (double)train_sequences[sequence_index].size();
					network->input->errors[i_index] = 0.0;
				}

				{
					vector<double> state(4, 0.0);
					for (int a_index = 0; a_index < (int)train_sequences[sequence_index].size(); a_index++) {
						int action = train_sequences[sequence_index][a_index];

						double ratio = 1.0 - (iter_index - 100000.0) / 400000.0;
						state[action] += ratio;

						vector<double> inputs = state;
						for (int s_index = 0; s_index < 4; s_index++) {
							if (action == s_index) {
								inputs.push_back(1.0);
							} else {
								inputs.push_back(0.0);
							}
						}
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
			if (iter_index >= 100000) {
				context_network->update();
			}
			network->update();
		}
	}

	// temp
	for (int h_index = 0; h_index < 40; h_index++) {
		cout << h_index << endl;

		vector<double> state(4, 0.0);
		for (int a_index = 0; a_index < (int)test_sequences[h_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			int action = test_sequences[h_index][a_index];
			cout << "action: " << action << endl;

			vector<double> inputs = state;
			for (int s_index = 0; s_index < 4; s_index++) {
				if (action == s_index) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			context_network->activate(inputs);
			for (int s_index = 0; s_index < 4; s_index++) {
				state[s_index] += context_network->output->acti_vals[s_index];
				cout << s_index << ": " << state[s_index] << endl;
			}
		}
		network->activate(state);

		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;

		cout << "test_target_vals[h_index]: " << test_target_vals[h_index] << endl;

		cout << endl;
	}

	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < (int)test_sequences.size(); h_index++) {
			vector<double> state(4, 0.0);
			for (int a_index = 0; a_index < (int)test_sequences[h_index].size(); a_index++) {
				int action = test_sequences[h_index][a_index];

				vector<double> inputs = state;
				for (int s_index = 0; s_index < 4; s_index++) {
					if (action == s_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				context_network->activate(inputs);
				for (int s_index = 0; s_index < 4; s_index++) {
					state[s_index] += context_network->output->acti_vals[s_index];
				}
			}
			network->activate(state);

			sum_misguess += (test_target_vals[h_index] - network->output->acti_vals[0])
				* (test_target_vals[h_index] - network->output->acti_vals[0]);
		}
		double average_misguess = sum_misguess / (double)test_sequences.size();
		cout << "average_misguess: " << average_misguess << endl;
	}

	cout << "Done" << endl;
}
