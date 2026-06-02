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

	vector<vector<vector<double>>> sequences_1;
	vector<vector<vector<double>>> sequences_2;
	vector<double> target_vals;

	uniform_int_distribution<int> xor_distribution(0, 1);
	uniform_int_distribution<int> starting_distribution(0, 3);
	for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
		vector<vector<double>> curr_sequence_1;
		int start_val_1;
		{
			vector<double> obs;
			if (starting_distribution(generator) == 0) {
				start_val_1 = 1.0;
			} else {
				start_val_1 = 0.0;
			}
			obs.push_back(start_val_1);
			obs.push_back(0.0);
			for (int i_index = 0; i_index < NUM_OBS-2; i_index++) {
				obs.push_back(xor_distribution(generator));
			}
			curr_sequence_1.push_back(obs);
		}
		for (int s_index = 1; s_index < SEQUENCE_LENGTH-1; s_index++) {
			vector<double> obs;
			obs.push_back(0.0);
			obs.push_back(0.0);
			for (int i_index = 0; i_index < NUM_OBS-2; i_index++) {
				obs.push_back(xor_distribution(generator));
			}
			curr_sequence_1.push_back(obs);
		}
		int end_val_1;
		{
			vector<double> obs;
			obs.push_back(0.0);
			end_val_1 = xor_distribution(generator);
			obs.push_back(end_val_1);
			for (int i_index = 0; i_index < NUM_OBS-2; i_index++) {
				obs.push_back(xor_distribution(generator));
			}
			curr_sequence_1.push_back(obs);
		}

		vector<vector<double>> curr_sequence_2;
		int start_val_2;
		{
			vector<double> obs;
			if (starting_distribution(generator) == 0) {
				start_val_2 = 1.0;
			} else {
				start_val_2 = 0.0;
			}
			obs.push_back(start_val_2);
			obs.push_back(0.0);
			for (int i_index = 0; i_index < NUM_OBS-2; i_index++) {
				obs.push_back(xor_distribution(generator));
			}
			curr_sequence_2.push_back(obs);
		}
		for (int s_index = 1; s_index < SEQUENCE_LENGTH-1; s_index++) {
			vector<double> obs;
			obs.push_back(0.0);
			obs.push_back(0.0);
			for (int i_index = 0; i_index < NUM_OBS-2; i_index++) {
				obs.push_back(xor_distribution(generator));
			}
			curr_sequence_2.push_back(obs);
		}
		int end_val_2;
		{
			vector<double> obs;
			obs.push_back(0.0);
			end_val_2 = xor_distribution(generator);
			obs.push_back(end_val_2);
			for (int i_index = 0; i_index < NUM_OBS-2; i_index++) {
				obs.push_back(xor_distribution(generator));
			}
			curr_sequence_2.push_back(obs);
		}

		double target_val = 0.0;
		if (start_val_1 == end_val_1) {
			target_val += 1.0;
		} else {
			target_val += -1.0;
		}
		if (start_val_2 == end_val_2) {
			target_val += 0.3;
		} else {
			target_val += -0.3;
		}

		sequences_1.push_back(curr_sequence_1);
		sequences_2.push_back(curr_sequence_2);
		target_vals.push_back(target_val);
	}

	Network* state_network_1 = new Network(LEAKY_LAYER, 5, 4);
	Network* final_network = new Network(LEAKY_LAYER, 4, 1);

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<NetworkHistory*> state_network_histories;

		vector<double> state(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences_1[sequence_index].size(); a_index++) {
			NetworkHistory* network_history = new NetworkHistory();
			state_network_1->activate(sequences_1[sequence_index][a_index],
									  network_history);
			state_network_histories.push_back(network_history);
			for (int s_index = 0; s_index < 4; s_index++) {
				state[s_index] += state_network_1->output->acti_vals[s_index];
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

		for (int h_index = (int)sequences_1[sequence_index].size()-1; h_index >= 0; h_index--) {
			state_network_1->backprop(state_errors,
									  state_network_histories[h_index]);
			delete state_network_histories[h_index];
		}

		if (iter_index % 20 == 0) {
			state_network_1->update();
			final_network->update();
		}
	}

	// temp
	for (int iter_index = 0; iter_index < 20; iter_index++) {
		cout << iter_index << endl;

		int sequence_index = sequence_distribution(generator);

		vector<double> state(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences_1[sequence_index].size(); a_index++) {
			state_network_1->activate(sequences_1[sequence_index][a_index]);
			for (int s_index = 0; s_index < 4; s_index++) {
				state[s_index] += state_network_1->output->acti_vals[s_index];
				// cout << s_index << ": " << state_network_1->output->acti_vals[s_index] << endl;
				cout << s_index << ": " << state[s_index] << endl;
			}
		}

		final_network->activate(state);

		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;
		cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;

		cout << endl;
	}
	// {
	// 	double sum_misguess = 0.0;
	// 	for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
	// 		vector<double> state(4, 0.0);
	// 		for (int a_index = 0; a_index < (int)sequences_1[h_index].size(); a_index++) {
	// 			state_network_1->activate(sequences_1[h_index][a_index]);
	// 			for (int s_index = 0; s_index < 4; s_index++) {
	// 				state[s_index] += state_network_1->output->acti_vals[s_index];
	// 			}
	// 		}

	// 		final_network->activate(state);

	// 		sum_misguess += (target_vals[h_index] - final_network->output->acti_vals[0])
	// 			* (target_vals[h_index] - final_network->output->acti_vals[0]);
	// 	}
	// 	double misguess_average = sum_misguess / (double)NUM_SAMPLES;
	// 	cout << "misguess_average: " << misguess_average << endl;
	// }

	Network* state_network_2 = new Network(LEAKY_LAYER, 5, 4);
	final_network->add_inputs(4);

	for (int iter_index = 0; iter_index < 20; iter_index++) {
		cout << iter_index << endl;

		int sequence_index = sequence_distribution(generator);

		vector<double> state_1(4, 0.0);
		vector<double> state_2(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences_1[sequence_index].size(); a_index++) {
			state_network_1->activate(sequences_1[sequence_index][a_index]);
			for (int s_index = 0; s_index < 4; s_index++) {
				state_1[s_index] += state_network_1->output->acti_vals[s_index];
			}

			state_network_2->activate(sequences_2[sequence_index][a_index]);
			for (int s_index = 0; s_index < 4; s_index++) {
				state_2[s_index] += state_network_2->output->acti_vals[s_index];
			}
		}

		vector<double> final_inputs;
		final_inputs.insert(final_inputs.end(), state_1.begin(), state_1.end());
		final_inputs.insert(final_inputs.end(), state_2.begin(), state_2.end());
		final_network->activate(final_inputs);

		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;
		cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;

		cout << endl;
	}

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<NetworkHistory*> state_network_histories;

		vector<double> state_1(4, 0.0);
		vector<double> state_2(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences_1[sequence_index].size(); a_index++) {
			state_network_1->activate(sequences_1[sequence_index][a_index]);
			for (int s_index = 0; s_index < 4; s_index++) {
				state_1[s_index] += state_network_1->output->acti_vals[s_index];
			}

			NetworkHistory* network_history = new NetworkHistory();
			state_network_2->activate(sequences_2[sequence_index][a_index],
									  network_history);
			state_network_histories.push_back(network_history);
			for (int s_index = 0; s_index < 4; s_index++) {
				state_2[s_index] += state_network_2->output->acti_vals[s_index];
			}
		}

		vector<double> final_inputs;
		final_inputs.insert(final_inputs.end(), state_1.begin(), state_1.end());
		final_inputs.insert(final_inputs.end(), state_2.begin(), state_2.end());
		final_network->activate(final_inputs);

		vector<double> final_errors{target_vals[sequence_index] - final_network->output->acti_vals[0]};
		final_network->backprop(final_errors);

		vector<double> state_errors(4, 0.0);
		for (int i_index = 0; i_index < 4; i_index++) {
			state_errors[i_index] += final_network->input->errors[4 + i_index];
			final_network->input->errors[4 + i_index] = 0.0;
		}

		for (int h_index = (int)sequences_1[sequence_index].size()-1; h_index >= 0; h_index--) {
			state_network_2->backprop(state_errors,
									  state_network_histories[h_index]);
			delete state_network_histories[h_index];
		}

		if (iter_index % 20 == 0) {
			state_network_2->update();
			final_network->update();
		}
	}

	for (int iter_index = 0; iter_index < 20; iter_index++) {
		cout << iter_index << endl;

		int sequence_index = sequence_distribution(generator);

		vector<double> state_1(4, 0.0);
		vector<double> state_2(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences_1[sequence_index].size(); a_index++) {
			state_network_1->activate(sequences_1[sequence_index][a_index]);
			for (int s_index = 0; s_index < 4; s_index++) {
				state_1[s_index] += state_network_1->output->acti_vals[s_index];
			}

			state_network_2->activate(sequences_2[sequence_index][a_index]);
			for (int s_index = 0; s_index < 4; s_index++) {
				state_2[s_index] += state_network_2->output->acti_vals[s_index];
			}
		}

		vector<double> final_inputs;
		final_inputs.insert(final_inputs.end(), state_1.begin(), state_1.end());
		final_inputs.insert(final_inputs.end(), state_2.begin(), state_2.end());
		final_network->activate(final_inputs);

		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;
		cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;

		cout << endl;
	}

	cout << "Done" << endl;
}
