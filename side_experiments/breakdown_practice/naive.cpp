// - plan is:
//   - break solution into parts
//   - turn parts into dimensions

// - on explore, check that:
//   - dimensions reach
//   - parts order/dependencies still satisfied

// - try not to improvise, but can use dimensions to try

// - there are many different ways to split and breakdown

// - some actions may be conditionally important
// - some actions may be conditionally safe

// - probably don't worry about combination eliminations
//   - have to order the sequence in the right way
//     - maybe involve removing more than 2 at once
//   - may be solved by dimensions/directions anyways

// - maybe just greedily split

// - duplicates making it less likely to have clean splits?
//   - even if you miss the target spot, there's redundancy in the back to get you to final target

// - if redundancy works, then means there can be multiple targets
//   - which suggests that everything can be treated as directions/dimensions?

// - maybe difficult to analyze multiple targets at once?
//   - at least without transforming into directions
//   - each is essentially a different solution
//     - to be combined, outer essentially has to make all the solution/target combinations into directions

// - there's like leeway
//   - a condition might have already been achieved, then there's some extra actions
//     - then, key can swap before those extra actions, but not before condition achieved

// - probably just eliminate redundancy, i.e., commit to one set of targets, and breakdown

// - goal is not to be able to anticipate everything that's safe?
//   - but just some things?

// - there may be some actions that really don't have any dependencies
//   - but are still needed
//     - should move these out of the way?
// - and other actions will depend on a subset of other actions

// - the way this works is:
//   - understand how current solution works
//   - explore in a way that still enables current solution and hope to find something new/additive

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "simple.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimple();

	// vector<int> initial_solution{
	// 	ACTION_DOWN,
	// 	ACTION_LEFT,
	// 	ACTION_UP,
	// 	ACTION_RIGHT,
	// 	ACTION_DOWN,
	// 	ACTION_RIGHT,

	// 	ACTION_DOWN,
	// 	ACTION_LEFT,
	// 	ACTION_DOWN,
	// 	ACTION_RIGHT,
	// 	/**
	// 	 * - the LEFT and RIGHT here also adding a lot of redundancy?
	// 	 *   - if instead only have 1, there would be more of a target?
	// 	 */
	// 	ACTION_DOWN,

	// 	ACTION_UP,
	// 	ACTION_DOWN,
	// 	ACTION_DOWN,
	// 	ACTION_DOWN,
	// 	ACTION_CLICK,

	// 	ACTION_UP,
	// 	ACTION_RIGHT
	// };

	vector<int> initial_solution{
		ACTION_DOWN,
		ACTION_LEFT,
		ACTION_UP,
		ACTION_RIGHT,
		ACTION_DOWN,
		ACTION_RIGHT,

		ACTION_DOWN,
		// ACTION_LEFT,
		ACTION_DOWN,
		ACTION_RIGHT,
		ACTION_DOWN,

		ACTION_UP,
		ACTION_DOWN,
		ACTION_DOWN,
		ACTION_DOWN,
		ACTION_CLICK,

		ACTION_UP,
		ACTION_RIGHT
	};

	// Problem* problem = problem_type->get_problem();

	// for (int a_index = 0; a_index < (int)initial_solution.size(); a_index++) {
	// 	problem->perform_action(initial_solution[a_index]);
	// }

	// double result = problem->score_result();
	// cout << "result: " << result << endl;

	// delete problem;

	vector<int> lower_bounds;
	for (int a_index = 0; a_index < (int)initial_solution.size(); a_index++) {
		int curr_bound = a_index;
		while (true) {
			if (curr_bound - 1 < 0) {
				break;
			}

			vector<int> test_solution = initial_solution;
			test_solution.erase(test_solution.begin() + a_index);
			test_solution.insert(test_solution.begin() + curr_bound-1, initial_solution[a_index]);

			Problem* problem = problem_type->get_problem();

			for (int a_index = 0; a_index < (int)test_solution.size(); a_index++) {
				problem->perform_action(test_solution[a_index]);
			}

			double result = problem->score_result();

			delete problem;

			if (result == 1.0) {
				curr_bound--;
			} else {
				break;
			}
		}
		lower_bounds.push_back(curr_bound);
	}

	cout << "lower_bounds:" << endl;
	for (int a_index = 0; a_index < (int)lower_bounds.size(); a_index++) {
		cout << a_index << ": " << lower_bounds[a_index] << endl;
	}

	vector<int> upper_bounds;
	for (int a_index = 0; a_index < (int)initial_solution.size(); a_index++) {
		int curr_bound = a_index;
		while (true) {
			if (curr_bound + 1 >= (int)initial_solution.size()) {
				break;
			}

			vector<int> test_solution = initial_solution;
			test_solution.insert(test_solution.begin() + curr_bound+1, initial_solution[a_index]);
			test_solution.erase(test_solution.begin() + a_index);

			Problem* problem = problem_type->get_problem();

			for (int a_index = 0; a_index < (int)test_solution.size(); a_index++) {
				problem->perform_action(test_solution[a_index]);
			}

			double result = problem->score_result();

			delete problem;

			if (result == 1.0) {
				curr_bound++;
			} else {
				break;
			}
		}
		upper_bounds.push_back(curr_bound);
	}

	cout << "upper_bounds:" << endl;
	for (int a_index = 0; a_index < (int)upper_bounds.size(); a_index++) {
		cout << a_index << ": " << upper_bounds[a_index] << endl;
	}

	delete problem_type;

	cout << "Done" << endl;
}
