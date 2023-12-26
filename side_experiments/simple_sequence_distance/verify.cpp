// TODO: split between random (or max distance?) and best potential score

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
		vector<int> action_sequence{0, 0, 2, 1};
		double target_val = -1.0;

		Try* new_try = new Try();
		new_try->sequence = action_sequence;
		new_try->result = target_val;

		try_tracker->tries.push_back(new_try);
	}
	{
		vector<int> action_sequence{0, 1, 0, 2, 0};
		double target_val = 1.0;

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

		{
			// verify
			Try* closest_match;
			double predicted_impact;
			vector<pair<int, int>> closest_diffs;
			try_tracker->evaluate_potential(new_try,
											closest_match,
											predicted_impact,
											closest_diffs);

			cout << "predicted_impact: " << predicted_impact << endl;
		}
	}

	delete try_tracker;

	cout << "Done" << endl;
}
