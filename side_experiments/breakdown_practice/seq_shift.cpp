// - use outer rules to clean

// - split by largest section, binary
// - after complete, merge outwards to create independent sections bigger than 2

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "breakdown_helpers.h"
#include "multi_level.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeMultiLevel();

	vector<int> initial_solution{
		MULTI_LEVEL_ACTION_DOWN,
		MULTI_LEVEL_ACTION_DOWN,
		MULTI_LEVEL_ACTION_DOWN,
		MULTI_LEVEL_ACTION_LEFT,
		MULTI_LEVEL_ACTION_UP,
		MULTI_LEVEL_ACTION_CLICK,

		MULTI_LEVEL_ACTION_RIGHT,
		MULTI_LEVEL_ACTION_RIGHT,
		MULTI_LEVEL_ACTION_LEFT,
		MULTI_LEVEL_ACTION_UP,
		MULTI_LEVEL_ACTION_UP,
		
		MULTI_LEVEL_ACTION_UP,
		MULTI_LEVEL_ACTION_UP,
		MULTI_LEVEL_ACTION_LEFT,
		MULTI_LEVEL_ACTION_UP,
		MULTI_LEVEL_ACTION_RIGHT,
		MULTI_LEVEL_ACTION_RIGHT,
		MULTI_LEVEL_ACTION_CLICK,

		MULTI_LEVEL_ACTION_DOWN,
		MULTI_LEVEL_ACTION_DOWN,
		MULTI_LEVEL_ACTION_RIGHT,
		MULTI_LEVEL_ACTION_LEFT,
		MULTI_LEVEL_ACTION_DOWN,
		MULTI_LEVEL_ACTION_DOWN,
	};

	/**
	 * - start_index, end_index, swap_index
	 */
	vector<vector<vector<bool>>> can_shift(initial_solution.size(),
		vector<vector<bool>>(initial_solution.size(),
			vector<bool>(initial_solution.size(), false)));

	for (int start_index = 0; start_index < (int)initial_solution.size(); start_index++) {
		for (int end_index = start_index+1; end_index < (int)initial_solution.size(); end_index++) {
			for (int pre_start_index = 0; pre_start_index < start_index; pre_start_index++) {
				vector<int> test_solution = initial_solution;

				test_solution.erase(test_solution.begin() + start_index, test_solution.begin() + end_index);
				test_solution.insert(test_solution.begin() + pre_start_index,
					initial_solution.begin() + start_index, initial_solution.begin() + end_index);

				Problem* problem = problem_type->get_problem();

				for (int a_index = 0; a_index < (int)test_solution.size(); a_index++) {
					problem->perform_action(test_solution[a_index]);
				}

				double result = problem->score_result();

				delete problem;

				if (result == 1.0) {
					can_shift[start_index][end_index][pre_start_index] = true;

					// temp
					cout << "start_index: " << start_index << endl;
					cout << "end_index: " << end_index << endl;
					cout << "pre_start_index: " << pre_start_index << endl;
					cout << "result: " << result << endl;
					cout << endl;
				}
			}

			// for (int post_end_index = end_index+1; post_end_index < (int)initial_solution.size(); post_end_index++) {

			// }
		}
	}

	delete problem_type;

	cout << "Done" << endl;
}
