/**
 * - kind of works
 * 
 * TOOD: try stabilizing too
 * 
 * - when adding state:
 *   - first, start with some event that may matter
 *     - add 1.0 each time event triggers, and train final network
 *   - then, train context, and gradually drop event?
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

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	for (int i_index = 0; i_index < NUM_SAMPLES; i_index++) {
		int num_actions = 1 + num_actions_distribution(generator);
		
		vector<int> curr_sequence;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
		}
		sequences.push_back(curr_sequence);

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

		if (sum_distance_one >= 1
				&& sum_distance_two >= 0) {
			target_vals.push_back(1.0);
		} else {
			target_vals.push_back(0.0);
		}
	}

	Network* context_network = new Network(SIGMOID_LAYER, 8, 4);
	// Network* context_network = new Network(LEAKY_LAYER, 8, 4);
	Network* network = new Network(SIGMOID_LAYER, 4, 1);
	// Network* network = new Network(LEAKY_LAYER, 4, 1);

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);
	// for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		int type;
		if (iter_index < 50000) {
			type = 0;
		} else {
			type = 1;
		}
		switch (type) {
		case 0:
			{
				vector<double> state(4, 0.0);
				for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
					int action = sequences[sequence_index][a_index];

					switch (action) {
					case 0:
					case 1:
						state[action] += 1.0;
						break;
					}
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

						switch (action) {
						case 0:
						case 1:
							state[action] += 1.0;
							break;
						}

						vector<double> inputs = state;
						for (int s_index = 0; s_index < 2; s_index++) {
							if (action == s_index) {
								inputs.push_back(1.0);
							} else {
								inputs.push_back(0.0);
							}
						}
						for (int s_index = 0; s_index < 2; s_index++) {
							inputs.push_back(0.0);
						}
						context_network->activate(inputs);
						for (int s_index = 0; s_index < 2; s_index++) {
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

						switch (action) {
						case 0:
						case 1:
							state[action] += 1.0;
							break;
						}

						vector<double> inputs = state;
						for (int s_index = 0; s_index < 2; s_index++) {
							if (action == s_index) {
								inputs.push_back(1.0);
							} else {
								inputs.push_back(0.0);
							}
						}
						for (int s_index = 0; s_index < 2; s_index++) {
							inputs.push_back(0.0);
						}
						context_network->activate(inputs);
						for (int s_index = 0; s_index < 2; s_index++) {
							state[s_index] += context_network->output->acti_vals[s_index];
						}
						context_network->backprop(errors);
					}
				}
			}
			break;
		}

		if (iter_index % 20 == 0) {
			if (iter_index >= 50000) {
				context_network->update();
			}
			network->update();
		}
	}

	// temp
	for (int h_index = 0; h_index < 40; h_index++) {
		cout << h_index << endl;

		vector<double> state(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			int action = sequences[h_index][a_index];
			cout << "action: " << action << endl;

			switch (action) {
			case 0:
			case 1:
				state[action] += 1.0;
				break;
			}

			vector<double> inputs = state;
			for (int s_index = 0; s_index < 2; s_index++) {
				if (action == s_index) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			for (int s_index = 0; s_index < 2; s_index++) {
				inputs.push_back(0.0);
			}
			context_network->activate(inputs);
			for (int s_index = 0; s_index < 2; s_index++) {
				state[s_index] += context_network->output->acti_vals[s_index];
				cout << s_index << ": " << state[s_index] << endl;
			}
		}
		network->activate(state);

		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < (int)sequences.size(); h_index++) {
			vector<double> state(4, 0.0);
			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				int action = sequences[h_index][a_index];

				switch (action) {
				case 0:
				case 1:
					state[action] += 1.0;
					break;
				}

				vector<double> inputs = state;
				for (int s_index = 0; s_index < 2; s_index++) {
					if (action == s_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				for (int s_index = 0; s_index < 2; s_index++) {
					inputs.push_back(0.0);
				}
				context_network->activate(inputs);
				for (int s_index = 0; s_index < 2; s_index++) {
					state[s_index] += context_network->output->acti_vals[s_index];
				}
			}
			network->activate(state);

			sum_misguess += (target_vals[h_index] - network->output->acti_vals[0])
				* (target_vals[h_index] - network->output->acti_vals[0]);
		}
		double average_misguess = sum_misguess / (double)sequences.size();
		cout << "average_misguess: " << average_misguess << endl;
	}

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

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
				context_network->activate(inputs);
				if (iter_index < 50000) {
					for (int s_index = 0; s_index < 2; s_index++) {
						state[s_index] += context_network->output->acti_vals[s_index];
					}
				} else {
					for (int s_index = 0; s_index < 4; s_index++) {
						state[s_index] += context_network->output->acti_vals[s_index];
					}
				}
			}
			network->activate(state);
		}

		vector<double> final_errors{target_vals[sequence_index] - network->output->acti_vals[0]};
		network->backprop(final_errors);

		vector<double> errors(4);
		if (iter_index < 50000) {
			for (int i_index = 0; i_index < 2; i_index++) {
				errors[i_index] = network->input->errors[i_index]
					/ (double)sequences[sequence_index].size();
				network->input->errors[i_index] = 0.0;
			}
			for (int i_index = 2; i_index < 4; i_index++) {
				errors[i_index] = 0.0;
				network->input->errors[i_index] = 0.0;
			}
		} else {
			for (int i_index = 0; i_index < 4; i_index++) {
				errors[i_index] = network->input->errors[i_index]
					/ (double)sequences[sequence_index].size();
				network->input->errors[i_index] = 0.0;
			}
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
				context_network->activate(inputs);
				if (iter_index < 50000) {
					for (int s_index = 0; s_index < 2; s_index++) {
						state[s_index] += context_network->output->acti_vals[s_index];
					}
				} else {
					for (int s_index = 0; s_index < 4; s_index++) {
						state[s_index] += context_network->output->acti_vals[s_index];
					}
				}
				context_network->backprop(errors);
			}
		}

		if (iter_index % 20 == 0) {
			context_network->update();
			network->update();
		}
	}

	// temp
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

	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < (int)sequences.size(); h_index++) {
			vector<double> state(4, 0.0);
			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				int action = sequences[h_index][a_index];

				state[action] += 1.0;

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

			sum_misguess += (target_vals[h_index] - network->output->acti_vals[0])
				* (target_vals[h_index] - network->output->acti_vals[0]);
		}
		double average_misguess = sum_misguess / (double)sequences.size();
		cout << "average_misguess: " << average_misguess << endl;
	}

	cout << "Done" << endl;
}
