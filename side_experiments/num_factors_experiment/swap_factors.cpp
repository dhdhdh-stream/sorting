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

const int NUM_SAMPLES = 10000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<int>> sequences;
	vector<vector<int>> contexts;
	vector<double> target_vals;

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	uniform_int_distribution<int> context_distribution(0, 9);
	// uniform_int_distribution<int> context_distribution(0, 4);
	for (int i_index = 0; i_index < NUM_SAMPLES; i_index++) {
		int num_actions = 1 + num_actions_distribution(generator);
		
		vector<int> curr_sequence;
		vector<int> curr_context;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
			if (context_distribution(generator) == 0) {
				curr_context.push_back(1);
			} else {
				curr_context.push_back(0);
			}
		}
		sequences.push_back(curr_sequence);
		contexts.push_back(curr_context);

		int sum_distance = 0;
		for (int a_index = 0; a_index < (int)curr_sequence.size(); a_index++) {
			switch (curr_sequence[a_index]) {
			case 0:
				if (curr_context[a_index] == 1) {
					sum_distance--;
				} else {
					sum_distance++;
				}
				break;
			case 1:
				if (curr_context[a_index] == 1) {
					sum_distance++;
				} else {
					sum_distance--;
				}
				break;
			}
		}

		if (sum_distance == 1) {
			target_vals.push_back(1.0);
		} else {
			target_vals.push_back(0.0);
		}
	}

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);

	Network* network = new Network(LEAKY_LAYER, 2, 1);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<double> state(2, 0.0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			int action = sequences[sequence_index][a_index];

			switch (action) {
			case 0:
				state[0] += 1.0;
				break;
			case 1:
				state[0] -= 1.0;
				break;
			}
		}

		network->activate(state);

		vector<double> final_errors{target_vals[sequence_index] - network->output->acti_vals[0]};
		network->backprop(final_errors);

		if (iter_index % 20 == 0) {
			network->update();
		}
	}

	// // temp
	// for (int iter_index = 0; iter_index < 40; iter_index++) {
	// 	cout << iter_index << endl;

	// 	int sequence_index = sequence_distribution(generator);

	// 	vector<double> state(2, 0.0);

	// 	for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
	// 		// cout << "a_index: " << a_index << endl;

	// 		int action = sequences[sequence_index][a_index];
	// 		// cout << "action: " << action << endl;

	// 		// cout << "contexts[sequence_index][a_index]: " << contexts[sequence_index][a_index] << endl;

	// 		switch (action) {
	// 		case 0:
	// 			state[0] += 1.0;
	// 			break;
	// 		case 1:
	// 			state[0] -= 1.0;
	// 			break;
	// 		}
	// 	}

	// 	network->activate(state);

	// 	cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	// 	cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;
	// }
	// temp
	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
			vector<double> state(2, 0.0);

			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				int action = sequences[h_index][a_index];

				switch (action) {
				case 0:
					state[0] += 1.0;
					break;
				case 1:
					state[0] -= 1.0;
					break;
				}
			}

			network->activate(state);

			sum_misguess += (target_vals[h_index] - network->output->acti_vals[0])
				* (target_vals[h_index] - network->output->acti_vals[0]);
		}
		double misguess_average = sum_misguess / (double)NUM_SAMPLES;
		cout << "misguess_average: " << misguess_average << endl;
	}

	// misguess_average: 0.0763854

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<double> state(2, 0.0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			int action = sequences[sequence_index][a_index];

			switch (action) {
			case 0:
				state[0] += 1.0;
				break;
			case 1:
				state[0] -= 1.0;
				break;
			}

			switch (action) {
			case 0:
				if (contexts[sequence_index][a_index] == 1) {
					state[1] -= 1.0;
				} else {
					state[1] += 1.0;
				}
				break;
			case 1:
				if (contexts[sequence_index][a_index] == 1) {
					state[1] += 1.0;
				} else {
					state[1] -= 1.0;
				}
				break;
			}
		}

		network->activate(state);

		vector<double> final_errors{target_vals[sequence_index] - network->output->acti_vals[0]};
		network->backprop(final_errors);

		if (iter_index % 20 == 0) {
			network->update();
		}
	}

	// // temp
	// for (int iter_index = 0; iter_index < 40; iter_index++) {
	// 	cout << iter_index << endl;

	// 	int sequence_index = sequence_distribution(generator);

	// 	vector<double> state(2, 0.0);

	// 	for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
	// 		// cout << "a_index: " << a_index << endl;

	// 		int action = sequences[sequence_index][a_index];
	// 		// cout << "action: " << action << endl;

	// 		// cout << "contexts[sequence_index][a_index]: " << contexts[sequence_index][a_index] << endl;

	// 		switch (action) {
	// 		case 0:
	// 			state[0] += 1.0;
	// 			break;
	// 		case 1:
	// 			state[0] -= 1.0;
	// 			break;
	// 		}

	// 		switch (action) {
	// 		case 0:
	// 			if (contexts[sequence_index][a_index] == 1) {
	// 				state[1] -= 1.0;
	// 			} else {
	// 				state[1] += 1.0;
	// 			}
	// 			break;
	// 		case 1:
	// 			if (contexts[sequence_index][a_index] == 1) {
	// 				state[1] += 1.0;
	// 			} else {
	// 				state[1] -= 1.0;
	// 			}
	// 			break;
	// 		}
	// 	}

	// 	network->activate(state);

	// 	cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	// 	cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;
	// }
	// temp
	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
			vector<double> state(2, 0.0);

			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				int action = sequences[h_index][a_index];

				switch (action) {
				case 0:
					state[0] += 1.0;
					break;
				case 1:
					state[0] -= 1.0;
					break;
				}

				switch (action) {
				case 0:
					if (contexts[h_index][a_index] == 1) {
						state[1] -= 1.0;
					} else {
						state[1] += 1.0;
					}
					break;
				case 1:
					if (contexts[h_index][a_index] == 1) {
						state[1] += 1.0;
					} else {
						state[1] -= 1.0;
					}
					break;
				}
			}

			network->activate(state);

			sum_misguess += (target_vals[h_index] - network->output->acti_vals[0])
				* (target_vals[h_index] - network->output->acti_vals[0]);
		}
		double misguess_average = sum_misguess / (double)NUM_SAMPLES;
		cout << "misguess_average: " << misguess_average << endl;
	}

	// misguess_average: 0.00325526

	cout << "Done" << endl;
}
