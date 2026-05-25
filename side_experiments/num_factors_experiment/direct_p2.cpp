// can sometimes work

// 0.131629
// 0.126829
// 0.115968
// 0.12157
// 0.108593
// 0.104842
// 0.127582
// 0.0754915
// 0.0647511
// 0.0638345
// 0.102935
// 0.0711192
// 0.118554
// 0.1165
// 0.126123

// naive is 0.1476

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
	// uniform_int_distribution<int> context_distribution(0, 9);
	uniform_int_distribution<int> context_distribution(0, 4);
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

	// // temp
	// int num_1s = 0;
	// for (int h_index = 0; h_index < (int)target_vals.size(); h_index++) {
	// 	if (target_vals[h_index] == 1.0) {
	// 		num_1s++;
	// 	}
	// }
	// cout << "num_1s: " << num_1s << endl;

	Network* final_network = new Network(LEAKY_LAYER, 4, 1);
	Network* state_one_network = new Network(LEAKY_LAYER, 4, 4);
	Network* state_two_existing_network = new Network(LEAKY_LAYER, 8, 4);
	Network* state_two_new_network = new Network(LEAKY_LAYER, 5, 4);

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<NetworkHistory*> state_one_network_histories;
		vector<NetworkHistory*> state_two_new_network_histories;
		vector<NetworkHistory*> state_two_existing_network_histories;

		vector<double> state_one(4, 0.0);
		vector<double> state_two(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			{
				vector<double> inputs = state_two;
				inputs.push_back((double)contexts[sequence_index][a_index]);
				NetworkHistory* network_history = new NetworkHistory();
				state_two_new_network->activate(inputs,
												network_history);
				state_two_new_network_histories.push_back(network_history);
				for (int s_index = 0; s_index < 4; s_index++) {
					state_two[s_index] += state_two_new_network->output->acti_vals[s_index];
				}
			}

			int action = sequences[sequence_index][a_index];

			{
				vector<double> inputs;
				for (int a_index = 0; a_index < 4; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				NetworkHistory* network_history = new NetworkHistory();
				state_one_network->activate(inputs,
											network_history);
				state_one_network_histories.push_back(network_history);
				for (int n_index = 0; n_index < 4; n_index++) {
					state_one[n_index] += state_one_network->output->acti_vals[n_index];
				}
			}

			{
				vector<double> inputs = state_two;
				for (int a_index = 0; a_index < 4; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				NetworkHistory* network_history = new NetworkHistory();
				state_two_existing_network->activate(inputs,
													 network_history);
				state_two_existing_network_histories.push_back(network_history);
				for (int n_index = 0; n_index < 4; n_index++) {
					state_one[n_index] += state_two_existing_network->output->acti_vals[n_index];
				}
			}
		}

		final_network->activate(state_one);

		vector<double> final_errors{target_vals[sequence_index] - final_network->output->acti_vals[0]};
		final_network->backprop(final_errors);

		vector<double> error_one(4, 0.0);
		for (int i_index = 0; i_index < 4; i_index++) {
			error_one[i_index] += final_network->input->errors[i_index];
			final_network->input->errors[i_index] = 0.0;
		}
		vector<double> error_two(4, 0.0);

		for (int h_index = (int)sequences[sequence_index].size()-1; h_index >= 0; h_index--) {
			state_one_network->backprop(error_one,
										state_one_network_histories[h_index]);
			delete state_one_network_histories[h_index];

			state_two_existing_network->backprop(error_one,
												 state_two_existing_network_histories[h_index]);
			delete state_two_existing_network_histories[h_index];
			for (int i_index = 0; i_index < 4; i_index++) {
				error_two[i_index] += state_two_existing_network->input->errors[i_index];
				state_two_existing_network->input->errors[i_index] = 0.0;
			}

			state_two_new_network->backprop(error_two,
											state_two_new_network_histories[h_index]);
			delete state_two_new_network_histories[h_index];
			for (int i_index = 0; i_index < 4; i_index++) {
				error_two[i_index] += state_two_new_network->input->errors[i_index];
				state_two_new_network->input->errors[i_index] = 0.0;
			}
		}

		if (iter_index % 20 == 0) {
			final_network->update();
			state_one_network->update();
			state_two_new_network->update();
			state_two_existing_network->update();
		}
	}

	// temp
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		int sequence_index = sequence_distribution(generator);

		vector<double> state_one(4, 0.0);
		vector<double> state_two(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			cout << "contexts[sequence_index][a_index]: " << contexts[sequence_index][a_index] << endl;

			{
				vector<double> inputs = state_two;
				inputs.push_back((double)contexts[sequence_index][a_index]);
				state_two_new_network->activate(inputs);
				cout << "state_two_new_network:" << endl;
				for (int s_index = 0; s_index < 4; s_index++) {
					state_two[s_index] += state_two_new_network->output->acti_vals[s_index];
					cout << s_index << ": " << state_two[s_index] << endl;
				}
			}

			int action = sequences[sequence_index][a_index];
			cout << "action: " << action << endl;

			{
				vector<double> inputs;
				for (int a_index = 0; a_index < 4; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				state_one_network->activate(inputs);
				cout << "state_one_network:" << endl;
				for (int n_index = 0; n_index < 4; n_index++) {
					state_one[n_index] += state_one_network->output->acti_vals[n_index];
					cout << n_index << ": " << state_one[n_index] << endl;
				}
			}

			{
				vector<double> inputs = state_two;
				for (int a_index = 0; a_index < 4; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				state_two_existing_network->activate(inputs);
				cout << "state_two_existing_network:" << endl;
				for (int n_index = 0; n_index < 4; n_index++) {
					state_one[n_index] += state_two_existing_network->output->acti_vals[n_index];
					cout << n_index << ": " << state_one[n_index] << endl;
				}
			}
		}

		final_network->activate(state_one);

		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;
		cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;

		cout << endl;
	}
	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
			vector<double> state_one(4, 0.0);
			vector<double> state_two(4, 0.0);
			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				{
					vector<double> inputs = state_two;
					inputs.push_back((double)contexts[h_index][a_index]);
					state_two_new_network->activate(inputs);
					for (int s_index = 0; s_index < 4; s_index++) {
						state_two[s_index] += state_two_new_network->output->acti_vals[s_index];
					}
				}

				int action = sequences[h_index][a_index];

				{
					vector<double> inputs;
					for (int a_index = 0; a_index < 4; a_index++) {
						if (action == a_index) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}

					state_one_network->activate(inputs);
					for (int n_index = 0; n_index < 4; n_index++) {
						state_one[n_index] += state_one_network->output->acti_vals[n_index];
					}
				}

				{
					vector<double> inputs = state_two;
					for (int a_index = 0; a_index < 4; a_index++) {
						if (action == a_index) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}

					state_two_existing_network->activate(inputs);
					for (int n_index = 0; n_index < 4; n_index++) {
						state_one[n_index] += state_two_existing_network->output->acti_vals[n_index];
					}
				}
			}

			final_network->activate(state_one);

			sum_misguess += (target_vals[h_index] - final_network->output->acti_vals[0])
				* (target_vals[h_index] - final_network->output->acti_vals[0]);
		}
		double misguess_average = sum_misguess / (double)NUM_SAMPLES;
		cout << "misguess_average: " << misguess_average << endl;
	}

	cout << "Done" << endl;
}
