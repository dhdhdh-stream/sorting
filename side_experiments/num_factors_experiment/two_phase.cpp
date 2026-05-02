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

const int NUM_SAMPLES = 1000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<int>> correct_sequences;
	vector<vector<int>> incorrect_sequences;

	geometric_distribution<int> num_actions_distribution(0.2);
	uniform_int_distribution<int> action_distribution(0, 3);
	while (true) {
		int num_actions = 1 + num_actions_distribution(generator);
		vector<int> curr_sequence;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
		}

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

		if (sum_distance == 2) {
			correct_sequences.push_back(curr_sequence);

			if (correct_sequences.size() >= NUM_SAMPLES) {
				break;
			}
		} else {
			if (incorrect_sequences.size() < NUM_SAMPLES) {
				incorrect_sequences.push_back(curr_sequence);
			}
		}
	}

	Network* network = new Network(4);
	Network* final_network = new Network(1);

	uniform_int_distribution<int> is_correct_distribution(0, 1);
	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);

	for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double sum_impact = 0.0;
		for (int a_index = 0; a_index < (int)correct_sequences[sequence_index].size(); a_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < 4; i_index++) {
				if (i_index == correct_sequences[sequence_index][a_index]) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			network->activate(inputs);

			sum_impact += network->output->acti_vals[0];
		}

		double average_error = (1.0 - sum_impact) / (double)correct_sequences[sequence_index].size();

		for (int a_index = 0; a_index < (int)correct_sequences[sequence_index].size(); a_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < 4; i_index++) {
				if (i_index == correct_sequences[sequence_index][a_index]) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			network->activate(inputs);

			network->backprop(average_error);
		}
	}

	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		if (is_correct_distribution(generator) == 0) {
			int sequence_index = sequence_distribution(generator);

			double sum_impact = 0.0;
			for (int a_index = 0; a_index < (int)correct_sequences[sequence_index].size(); a_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < 4; i_index++) {
					if (i_index == correct_sequences[sequence_index][a_index]) {
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

			double final_error;
			if (final_network->output->acti_vals[0] >= 1.0) {
				final_error = 0.0;
			} else {
				final_error = 1.0 - final_network->output->acti_vals[0];
			}
			// double final_error = 1.0 - final_network->output->acti_vals[0];
			final_network->backprop(final_error);

			double error = final_network->input->errors[0];
			final_network->input->errors[0] = 0.0;
			double average_error = error / (double)correct_sequences[sequence_index].size();
			// temp
			if (iter_index % 1000 == 0) {
				cout << "average_error: " << average_error << endl;
			}

			for (int a_index = 0; a_index < (int)correct_sequences[sequence_index].size(); a_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < 4; i_index++) {
					if (i_index == correct_sequences[sequence_index][a_index]) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				network->activate(inputs);

				network->backprop(average_error);
			}
		} else {
			int sequence_index = sequence_distribution(generator);

			double sum_impact = 0.0;
			for (int a_index = 0; a_index < (int)incorrect_sequences[sequence_index].size(); a_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < 4; i_index++) {
					if (i_index == incorrect_sequences[sequence_index][a_index]) {
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

			double final_error;
			if (final_network->output->acti_vals[0] <= -1.0) {
				final_error = 0.0;
			} else {
				final_error = -1.0 - final_network->output->acti_vals[0];
			}
			// double final_error = -1.0 - final_network->output->acti_vals[0];
			final_network->backprop(final_error);

			double error = final_network->input->errors[0];
			final_network->input->errors[0] = 0.0;
			double average_error = error / (double)incorrect_sequences[sequence_index].size();
			// temp
			if (iter_index % 1000 == 0) {
				cout << "average_error: " << average_error << endl;
			}

			for (int a_index = 0; a_index < (int)incorrect_sequences[sequence_index].size(); a_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < 4; i_index++) {
					if (i_index == incorrect_sequences[sequence_index][a_index]) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				network->activate(inputs);

				network->backprop(average_error);
			}
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

	cout << "correct_sequences:" << endl;
	for (int h_index = 0; h_index < 10; h_index++) {
		cout << h_index << endl;

		double sum_impact = 0.0;
		for (int a_index = 0; a_index < (int)correct_sequences[h_index].size(); a_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < 4; i_index++) {
				if (i_index == correct_sequences[h_index][a_index]) {
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
	}

	cout << "incorrect_sequences:" << endl;
	for (int h_index = 0; h_index < 10; h_index++) {
		cout << h_index << endl;

		double sum_impact = 0.0;
		for (int a_index = 0; a_index < (int)incorrect_sequences[h_index].size(); a_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < 4; i_index++) {
				if (i_index == incorrect_sequences[h_index][a_index]) {
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
	}

	cout << "Done" << endl;
}
