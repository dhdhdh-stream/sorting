// - works pretty well

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

const int NUM_TARGET_INCORRECT = 500;
const int NUM_TARGET_INCORRECT_TRIES = 10000;
const int NUM_TOTAL = 1000;

const double DIFF_THRESHOLD = 0.2;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* network_one = new Network(6);
	Network* network_two = new Network(6);
	Network* network_three = new Network(6);

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 5);
	for (int epoch_index = 0; epoch_index < 20; epoch_index++) {
		vector<vector<int>> correct_sequences;
		while (true) {
			int num_actions = 1 + num_actions_distribution(generator);
			vector<int> curr_sequence;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				curr_sequence.push_back(action_distribution(generator));
			}

			int sum_distance_one = 0;
			int sum_distance_two = 0;
			int sum_distance_three = 0;
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
				case 4:
					sum_distance_three--;
					break;
				case 5:
					sum_distance_three++;
					break;
				}
			}

			if (sum_distance_one == 2
					&& sum_distance_two == 1
					&& sum_distance_three == -1) {
				correct_sequences.push_back(curr_sequence);

				if (correct_sequences.size() >= NUM_TOTAL) {
					break;
				}
			}
		}

		vector<vector<int>> incorrect_sequences;
		for (int iter_index = 0; iter_index < NUM_TARGET_INCORRECT_TRIES; iter_index++) {
			int num_actions = 1 + num_actions_distribution(generator);
			vector<int> curr_sequence;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				curr_sequence.push_back(action_distribution(generator));
			}

			int sum_distance_one = 0;
			int sum_distance_two = 0;
			int sum_distance_three = 0;
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
				case 4:
					sum_distance_three--;
					break;
				case 5:
					sum_distance_three++;
					break;
				}
			}

			if (sum_distance_one != 2
					|| sum_distance_two != 1
					|| sum_distance_three != -1) {
				double sum_impact_one = 0.0;
				double sum_impact_two = 0.0;
				double sum_impact_three = 0.0;
				for (int a_index = 0; a_index < (int)curr_sequence.size(); a_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < 6; i_index++) {
						if (i_index == curr_sequence[a_index]) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}
					network_one->activate(inputs);
					network_two->activate(inputs);
					network_three->activate(inputs);

					sum_impact_one += network_one->output->acti_vals[0];
					sum_impact_two += network_two->output->acti_vals[0];
					sum_impact_three += network_three->output->acti_vals[0];
				}

				if (sum_impact_one >= 1.0 - DIFF_THRESHOLD
						&& sum_impact_one <= 1.0 + DIFF_THRESHOLD
						&& sum_impact_two >= 1.0 - DIFF_THRESHOLD
						&& sum_impact_two <= 1.0 + DIFF_THRESHOLD
						&& sum_impact_three >= 1.0 - DIFF_THRESHOLD
						&& sum_impact_three <= 1.0 + DIFF_THRESHOLD) {
					incorrect_sequences.push_back(curr_sequence);

					if (incorrect_sequences.size() >= NUM_TARGET_INCORRECT) {
						break;
					}
				}
			}
		}
		// temp
		cout << "incorrect_sequences.size(): " << incorrect_sequences.size() << endl;
		while (true) {
			int num_actions = 1 + num_actions_distribution(generator);
			vector<int> curr_sequence;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				curr_sequence.push_back(action_distribution(generator));
			}

			int sum_distance_one = 0;
			int sum_distance_two = 0;
			int sum_distance_three = 0;
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
				case 4:
					sum_distance_three--;
					break;
				case 5:
					sum_distance_three++;
					break;
				}
			}

			if (sum_distance_one != 2
					|| sum_distance_two != 1
					|| sum_distance_three != -1) {
				incorrect_sequences.push_back(curr_sequence);

				if (incorrect_sequences.size() >= NUM_TOTAL) {
					break;
				}
			}
		}

		uniform_int_distribution<int> is_correct_distribution(0, 1);
		uniform_int_distribution<int> sequence_distribution(0, NUM_TOTAL-1);
		for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
			if (iter_index % 10000 == 0) {
				cout << iter_index << endl;
			}

			if (is_correct_distribution(generator) == 0) {
				int sequence_index = sequence_distribution(generator);

				double sum_impact_one = 0.0;
				double sum_impact_two = 0.0;
				double sum_impact_three = 0.0;
				for (int a_index = 0; a_index < (int)correct_sequences[sequence_index].size(); a_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < 6; i_index++) {
						if (i_index == correct_sequences[sequence_index][a_index]) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}
					network_one->activate(inputs);
					network_two->activate(inputs);
					network_three->activate(inputs);

					sum_impact_one += network_one->output->acti_vals[0];
					sum_impact_two += network_two->output->acti_vals[0];
					sum_impact_three += network_three->output->acti_vals[0];
				}

				double average_error_one = (1.0 - sum_impact_one) / (double)correct_sequences[sequence_index].size();
				double average_error_two = (1.0 - sum_impact_two) / (double)correct_sequences[sequence_index].size();
				double average_error_three = (1.0 - sum_impact_three) / (double)correct_sequences[sequence_index].size();

				for (int a_index = 0; a_index < (int)correct_sequences[sequence_index].size(); a_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < 6; i_index++) {
						if (i_index == correct_sequences[sequence_index][a_index]) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}
					network_one->activate(inputs);
					network_two->activate(inputs);
					network_three->activate(inputs);

					network_one->backprop(average_error_one);
					network_two->backprop(average_error_two);
					network_three->backprop(average_error_three);
				}
			} else {
				int sequence_index = sequence_distribution(generator);

				double sum_impact_one = 0.0;
				double sum_impact_two = 0.0;
				double sum_impact_three = 0.0;
				for (int a_index = 0; a_index < (int)correct_sequences[sequence_index].size(); a_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < 6; i_index++) {
						if (i_index == correct_sequences[sequence_index][a_index]) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}
					network_one->activate(inputs);
					network_two->activate(inputs);
					network_three->activate(inputs);

					sum_impact_one += network_one->output->acti_vals[0];
					sum_impact_two += network_two->output->acti_vals[0];
					sum_impact_three += network_three->output->acti_vals[0];
				}

				double error_one = 0.0;
				if (sum_impact_one > 1.0 && sum_impact_one < 1.0 + DIFF_THRESHOLD) {
					error_one = 1.0 + DIFF_THRESHOLD - sum_impact_one;
				} else if (sum_impact_one < 1.0 && sum_impact_one > 1.0 - DIFF_THRESHOLD) {
					error_one = 1.0 - DIFF_THRESHOLD - sum_impact_one;
				}
				double error_two = 0.0;
				if (sum_impact_two > 1.0 && sum_impact_two < 1.0 + DIFF_THRESHOLD) {
					error_two = 1.0 + DIFF_THRESHOLD - sum_impact_two;
				} else if (sum_impact_two < 1.0 && sum_impact_two > 1.0 - DIFF_THRESHOLD) {
					error_two = 1.0 - DIFF_THRESHOLD - sum_impact_two;
				}
				double error_three = 0.0;
				if (sum_impact_three > 1.0 && sum_impact_three < 1.0 + DIFF_THRESHOLD) {
					error_three = 1.0 + DIFF_THRESHOLD - sum_impact_three;
				} else if (sum_impact_three < 1.0 && sum_impact_three > 1.0 - DIFF_THRESHOLD) {
					error_three = 1.0 - DIFF_THRESHOLD - sum_impact_three;
				}

				double average_error_one = error_one / (double)incorrect_sequences[sequence_index].size();
				double average_error_two = error_two / (double)incorrect_sequences[sequence_index].size();
				double average_error_three = error_three / (double)incorrect_sequences[sequence_index].size();

				for (int a_index = 0; a_index < (int)incorrect_sequences[sequence_index].size(); a_index++) {
					vector<double> inputs;
					for (int i_index = 0; i_index < 6; i_index++) {
						if (i_index == incorrect_sequences[sequence_index][a_index]) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}
					network_one->activate(inputs);
					network_two->activate(inputs);
					network_three->activate(inputs);

					network_one->backprop(average_error_one);
					network_two->backprop(average_error_two);
					network_three->backprop(average_error_three);
				}
			}
		}

		for (int a_index = 0; a_index < 6; a_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < 6; i_index++) {
				if (i_index == a_index) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			network_one->activate(inputs);
			network_two->activate(inputs);
			network_three->activate(inputs);

			cout << a_index << endl;
			cout << "one: " << network_one->output->acti_vals[0] << endl;
			cout << "two: " << network_two->output->acti_vals[0] << endl;
			cout << "three: " << network_three->output->acti_vals[0] << endl;
		}

		cout << "correct_sequences:" << endl;
		for (int h_index = 0; h_index < 10; h_index++) {
			cout << h_index << endl;

			double sum_impact_one = 0.0;
			double sum_impact_two = 0.0;
			double sum_impact_three = 0.0;
			for (int a_index = 0; a_index < (int)correct_sequences[h_index].size(); a_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < 6; i_index++) {
					if (i_index == correct_sequences[h_index][a_index]) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				network_one->activate(inputs);
				network_two->activate(inputs);
				network_three->activate(inputs);

				sum_impact_one += network_one->output->acti_vals[0];
				sum_impact_two += network_two->output->acti_vals[0];
				sum_impact_three += network_three->output->acti_vals[0];
			}

			cout << "sum_impact_one: " << sum_impact_one << endl;
			cout << "sum_impact_two: " << sum_impact_two << endl;
			cout << "sum_impact_three: " << sum_impact_three << endl;
		}

		cout << "incorrect_sequences:" << endl;
		for (int h_index = 0; h_index < 10; h_index++) {
			cout << h_index << endl;

			double sum_impact_one = 0.0;
			double sum_impact_two = 0.0;
			double sum_impact_three = 0.0;
			for (int a_index = 0; a_index < (int)incorrect_sequences[h_index].size(); a_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < 6; i_index++) {
					if (i_index == incorrect_sequences[h_index][a_index]) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				network_one->activate(inputs);
				network_two->activate(inputs);
				network_three->activate(inputs);

				sum_impact_one += network_one->output->acti_vals[0];
				sum_impact_two += network_two->output->acti_vals[0];
				sum_impact_three += network_three->output->acti_vals[0];
			}

			cout << "sum_impact_one: " << sum_impact_one << endl;
			cout << "sum_impact_two: " << sum_impact_two << endl;
			cout << "sum_impact_three: " << sum_impact_three << endl;
		}
	}

	cout << "Done" << endl;
}
