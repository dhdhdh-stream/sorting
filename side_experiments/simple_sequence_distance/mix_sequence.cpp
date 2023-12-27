// this setup doesn't pay attention to locality well

// RNNs don't work well either
// - or at least will take a lot of fiddling with the number of states to save, etc.
//   - and not reliable, requires training time, etc.

// the reason why doesn't pay attention to locality well is because of duplicates
// - actions are ahead and behind, and also ahead multiple times, etc.
//   - makes it difficult to clearly identify sequences/ordering

// - like, if want ABC, something like BABC makes things difficult
//   - or actually, BACB is indistinguishable from ABC
//     - also ABDC is not indistinguisable

// - but actually, scopes/compound actions capture locality
//   - so maybe don't need analysis to be good at it as well

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
	uniform_int_distribution<int> type_distribution(0, 2);
	for (int iter_index = 0; iter_index < 500; iter_index++) {
		int type = type_distribution(generator);
		if (type == 0) {
			// random
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
			try_tracker->backprop(new_try,
								  closest_match,
								  predicted_impact,
								  closest_diffs);

			// for (int i = 0; i < 5; i++) {
			// 	try_tracker->random_backprop(new_try);
			// }

			try_tracker->tries.push_back(new_try);
		} else if (type == 1) {
			// furthest distance
			int instance_length = 1 + geometric_distribution(generator);

			Try* furthest_try = NULL;
			Try* furthest_closest_match;
			double furthest_predicted_impact;
			vector<pair<int, int>> furthest_diffs;
			for (int t_index = 0; t_index < 10; t_index++) {
				vector<int> action_sequence;
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

				if (furthest_try == NULL) {
					furthest_try = new_try;
					furthest_closest_match = closest_match;
					furthest_predicted_impact = predicted_impact;
					furthest_diffs = closest_diffs;
				} else if (furthest_diffs.size() < closest_diffs.size()) {
					delete furthest_try;

					furthest_try = new_try;
					furthest_closest_match = closest_match;
					furthest_predicted_impact = predicted_impact;
					furthest_diffs = closest_diffs;
				}
			}

			try_tracker->backprop(furthest_try,
								  furthest_closest_match,
								  furthest_predicted_impact,
								  furthest_diffs);

			// for (int i = 0; i < 5; i++) {
			// 	try_tracker->random_backprop(furthest_try);
			// }

			try_tracker->tries.push_back(furthest_try);
		} else {
			// highest potential
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

			// for (int i = 0; i < 5; i++) {
			// 	try_tracker->random_backprop(best_try);
			// }

			try_tracker->tries.push_back(best_try);
		}

		for (int iter_index = 0; iter_index < 10; iter_index++) {
			try_tracker->random_backprop();
		}
	}

	for (int iter_index = 0; iter_index < 10000; iter_index++) {
		try_tracker->random_backprop();
	}

	// highest potential
	for (int iter_index = 0; iter_index < 20; iter_index++) {
		Try* best_try = NULL;
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
				best_predicted_impact = predicted_impact;
				best_diffs = closest_diffs;
			} else if (best_predicted_impact < predicted_impact) {
				delete best_try;

				best_try = new_try;
				best_predicted_impact = predicted_impact;
				best_diffs = closest_diffs;
			}
		}

		cout << "best_try:" << endl;
		for (int a_index = 0; a_index < (int)best_try->sequence.size(); a_index++) {
			cout << best_try->sequence[a_index] << " ";
		}
		cout << endl;

		Try* closest_match;
		double predicted_impact;
		try_tracker->verify_potential(best_try,
									  closest_match,
									  predicted_impact);
		cout << "closest_match:";
		for (int a_index = 0; a_index < (int)closest_match->sequence.size(); a_index++) {
			cout << " " << closest_match->sequence[a_index];
		}
		cout << endl;

		cout << "predicted_impact: " << predicted_impact << endl;

		cout << endl;

		delete best_try;
	}

	delete try_tracker;

	cout << "Done" << endl;
}
