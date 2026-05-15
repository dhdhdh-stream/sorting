#include <algorithm>
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

const int NUM_POTENTIAL = 1000;
const int NUM_SELECTED = 100;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<int>> sequences;
	vector<double> target_vals;

	Network* network = new Network(LEAKY_LAYER, 4, 1);

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	
	for (int epoch_index = 0; epoch_index < 10; epoch_index++) {
		vector<pair<double, pair<double,vector<int>>>> potential_sequences;

		for (int i_index = 0; i_index < NUM_POTENTIAL; i_index++) {
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

			double target_val;
			if (sum_distance == 1) {
				target_val = 1.0;
			} else {
				target_val = 0.0;
			}

			vector<double> state(4, 0.0);
			for (int a_index = 0; a_index < (int)curr_sequence.size(); a_index++) {
				state[curr_sequence[a_index]] += 1.0;
			}
			network->activate(state);
			double predicted = network->output->acti_vals[0];

			potential_sequences.push_back({predicted, {target_val, curr_sequence}});
		}
		sort(potential_sequences.begin(), potential_sequences.end());

		for (int i_index = 0; i_index < NUM_SELECTED; i_index++) {
			sequences.push_back(potential_sequences[potential_sequences.size()-1 - i_index].second.second);
			double target_val = potential_sequences[potential_sequences.size()-1 - i_index].second.first;
			cout << "target_val: " << target_val << endl;
			target_vals.push_back(target_val);

			double predicted = potential_sequences[potential_sequences.size()-1 - i_index].first;
			cout << "predicted: " << predicted << endl;
		}

		uniform_int_distribution<int> sequence_distribution(0, sequences.size()-1);
		for (int iter_index = 0; iter_index < 200000; iter_index++) {
			if (iter_index % 10000 == 0) {
				cout << iter_index << endl;
			}

			int sequence_index = sequence_distribution(generator);

			vector<double> state(4, 0.0);
			for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
				state[sequences[sequence_index][a_index]] += 1.0;
			}
			network->activate(state);

			vector<double> errors{target_vals[sequence_index] - network->output->acti_vals[0]};
			network->backprop(errors);

			if (iter_index % 20 == 0) {
				network->update();
			}
		}

		// // temp
		// for (int h_index = 0; h_index < 10; h_index++) {
		// 	cout << h_index << endl;

		// 	vector<double> state(4, 0.0);
		// 	for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
		// 		cout << "a_index: " << a_index << endl;

		// 		int action = sequences[h_index][a_index];
		// 		cout << "action: " << action << endl;

		// 		state[action] += 1.0;
		// 	}
		// 	network->activate(state);

		// 	cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;

		// 	cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		// 	cout << endl;
		// }

		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < (int)sequences.size(); h_index++) {
			vector<double> state(4, 0.0);
			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				state[sequences[h_index][a_index]] += 1.0;
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
