#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "try.h"
#include "try_tracker.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	TryTracker* try_tracker = new TryTracker();

	geometric_distribution<int> geometric_distribution(0.3);
	uniform_int_distribution<int> action_distribution(0, 2);
	{
		vector<int> action_sequence;
		int instance_length = 1 + geometric_distribution(generator);
		for (int l_index = 0; l_index < instance_length; l_index++) {
			int action = action_distribution(generator);
			action_sequence.push_back(action);
		}

		bool found_sequence = false;
		for (int a_index = 0; a_index < (int)action_sequence.size()-2; a_index++) {
			if (action_sequence[a_index] == 0
					&& action_sequence[a_index+1] == 1
					&& action_sequence[a_index+2] == 1) {
				found_sequence = true;
			}
		}

		double target_val;
		if (found_sequence) {
			target_val = 1.0;
		} else {
			target_val = -1.0;
		}

		Try* new_try = new Try();
		new_try->sequence = action_sequence;
		new_try->result = target_val;

		try_tracker->tries.push_back(new_try);
	}
	for (int iter_index = 0; iter_index < 200; iter_index++) {
		Try* best_try = NULL;
		Try* best_closest_match;
		double best_predicted_impact;
		vector<pair<int, int>> best_diffs;
		for (int t_index = 0; t_index < 10; t_index++) {
			vector<int> action_sequence;
			int instance_length = 1 + geometric_distribution(generator);
			for (int l_index = 0; l_index < instance_length; l_index++) {
				int action = action_distribution(generator);
				action_sequence.push_back(action);
			}

			bool found_sequence = false;
			for (int a_index = 0; a_index < (int)action_sequence.size()-2; a_index++) {
				if (action_sequence[a_index] == 0
						&& action_sequence[a_index+1] == 1
						&& action_sequence[a_index+2] == 1) {
					found_sequence = true;
				}
			}

			double target_val;
			if (found_sequence) {
				target_val = 1.0;
			} else {
				target_val = -1.0;
			}

			Try* new_try = new Try();
			new_try->sequence = action_sequence;
			new_try->result = target_val;

			Try* closest_match;
			double predicted_impact;
			vector<pair<int, int>> closest_diffs;
			try_tracker->evaluate_potential(new_try,
											closest_match,
											predicted_impact,
											closest_diffs);

			if (best_try == NULL) {
				best_try = new_try;
				best_closest_match = closest_match;
				best_predicted_impact = predicted_impact;
				best_diffs = closest_diffs;
			} else if (best_predicted_impact < predicted_impact) {
				delete best_try;

				best_try = new_try;
				best_closest_match = closest_match;
				best_predicted_impact = predicted_impact;
				best_diffs = closest_diffs;
			}
		}

		try_tracker->backprop(best_try,
							  best_closest_match,
							  best_predicted_impact,
							  best_diffs);

		try_tracker->tries.push_back(best_try);
	}

	for (int iter_index = 0; iter_index < 100; iter_index++) {
		vector<int> action_sequence;
		int instance_length = 1 + geometric_distribution(generator);
		for (int l_index = 0; l_index < instance_length; l_index++) {
			int action = action_distribution(generator);
			action_sequence.push_back(action);
		}

		bool found_sequence = false;
		for (int a_index = 0; a_index < (int)action_sequence.size()-2; a_index++) {
			if (action_sequence[a_index] == 0
					&& action_sequence[a_index+1] == 1
					&& action_sequence[a_index+2] == 1) {
				found_sequence = true;
			}
		}

		double target_val;
		if (found_sequence) {
			target_val = 1.0;
		} else {
			target_val = -1.0;
		}

		cout << iter_index << endl;
		for (int a_index = 0; a_index < (int)action_sequence.size(); a_index++) {
			cout << action_sequence[a_index] << " ";
		}
		cout << endl;
		cout << "target_val: " << target_val << endl;

		Try* new_try = new Try();
		new_try->sequence = action_sequence;
		new_try->result = target_val;

		Try* closest_match;
		double predicted_impact;
		vector<pair<int, int>> closest_diffs;
		try_tracker->evaluate_potential(new_try,
										closest_match,
										predicted_impact,
										closest_diffs);
		cout << "closest_match:";
		for (int a_index = 0; a_index < (int)closest_match->sequence.size(); a_index++) {
			cout << " " << closest_match->sequence[a_index];
		}
		cout << endl;

		cout << "predicted_impact: " << predicted_impact << endl;

		cout << endl;

		delete new_try;
	}

	delete try_tracker;

	cout << "Done" << endl;
}
