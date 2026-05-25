// - this state-based setup is what the goal is
//   - but unlikely to get there from random
//     - instead, try to get there from breaking down solution?

// - does work actually

// TODO: try double XOR and with self-ref

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

const int NUM_OBS = 5;
const int SEQUENCE_LENGTH = 10;

const int NUM_SAMPLES = 10000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<vector<double>>> sequences;
	vector<double> target_vals;

	uniform_int_distribution<int> xor_distribution(0, 1);
	// temp
	uniform_int_distribution<int> starting_distribution(0, 3);
	for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
		vector<vector<double>> curr_sequence;
		// int start_val;
		// {
		// 	vector<double> obs;
		// 	// start_val = xor_distribution(generator);
		// 	if (starting_distribution(generator) == 0) {
		// 		start_val = 1.0;
		// 	} else {
		// 		start_val = 0.0;
		// 	}
		// 	obs.push_back(start_val);
		// 	obs.push_back(0.0);
		// 	for (int i_index = 0; i_index < NUM_OBS-2; i_index++) {
		// 		obs.push_back(xor_distribution(generator));
		// 	}
		// 	curr_sequence.push_back(obs);
		// }
		for (int s_index = 1; s_index < SEQUENCE_LENGTH-1; s_index++) {
			vector<double> obs;
			obs.push_back(0.0);
			obs.push_back(0.0);
			for (int i_index = 0; i_index < NUM_OBS-2; i_index++) {
				obs.push_back(xor_distribution(generator));
			}
			curr_sequence.push_back(obs);
		}
		int start_val;
		{
			vector<double> obs;
			// start_val = xor_distribution(generator);
			if (starting_distribution(generator) == 0) {
				start_val = 1.0;
			} else {
				start_val = 0.0;
			}
			obs.push_back(start_val);
			obs.push_back(0.0);
			for (int i_index = 0; i_index < NUM_OBS-2; i_index++) {
				obs.push_back(xor_distribution(generator));
			}
			curr_sequence.push_back(obs);
		}
		int end_val;
		{
			vector<double> obs;
			obs.push_back(0.0);
			end_val = xor_distribution(generator);
			obs.push_back(end_val);
			for (int i_index = 0; i_index < NUM_OBS-2; i_index++) {
				obs.push_back(xor_distribution(generator));
			}
			curr_sequence.push_back(obs);
		}

		double target_val;
		if (start_val == end_val) {
			target_val = 1.0;
		} else {
			target_val = -1.0;
		}

		sequences.push_back(curr_sequence);
		target_vals.push_back(target_val);
	}

	Network* state_network = new Network(LEAKY_LAYER, 5, 4);
	Network* final_network = new Network(LEAKY_LAYER, 4, 1);

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<NetworkHistory*> state_network_histories;

		vector<double> state(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			NetworkHistory* network_history = new NetworkHistory();
			state_network->activate(sequences[sequence_index][a_index],
									network_history);
			state_network_histories.push_back(network_history);
			for (int s_index = 0; s_index < 4; s_index++) {
				state[s_index] += state_network->output->acti_vals[s_index];
			}
		}

		final_network->activate(state);

		vector<double> final_errors{target_vals[sequence_index] - final_network->output->acti_vals[0]};
		final_network->backprop(final_errors);

		vector<double> state_errors(4, 0.0);
		for (int i_index = 0; i_index < 4; i_index++) {
			state_errors[i_index] += final_network->input->errors[i_index];
			final_network->input->errors[i_index] = 0.0;
		}

		for (int h_index = (int)sequences[sequence_index].size()-1; h_index >= 0; h_index--) {
			state_network->backprop(state_errors,
									state_network_histories[h_index]);
			delete state_network_histories[h_index];
		}

		if (iter_index % 20 == 0) {
			state_network->update();
			final_network->update();
		}
	}

	// temp
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		int sequence_index = sequence_distribution(generator);

		vector<double> state(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			state_network->activate(sequences[sequence_index][a_index]);
			for (int s_index = 0; s_index < 4; s_index++) {
				state[s_index] += state_network->output->acti_vals[s_index];
				// cout << s_index << ": " << state_network->output->acti_vals[s_index] << endl;
				cout << s_index << ": " << state[s_index] << endl;
			}
		}

		final_network->activate(state);

		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;
		cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;

		cout << endl;
	}
	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
			vector<double> state(4, 0.0);
			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				state_network->activate(sequences[h_index][a_index]);
				for (int s_index = 0; s_index < 4; s_index++) {
					state[s_index] += state_network->output->acti_vals[s_index];
				}
			}

			final_network->activate(state);

			sum_misguess += (target_vals[h_index] - final_network->output->acti_vals[0])
				* (target_vals[h_index] - final_network->output->acti_vals[0]);
		}
		double misguess_average = sum_misguess / (double)NUM_SAMPLES;
		cout << "misguess_average: " << misguess_average << endl;
	}

	cout << "Done" << endl;
}
