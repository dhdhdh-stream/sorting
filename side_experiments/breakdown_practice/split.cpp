// - shifting 1 is not the same as full independence
//   - but testing full independence requires exponential tries
//   - so maybe not a big deal?
//     - as long as keep in mind limitation
//       - I mean, actions are going to be context dependent anyways?

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "breakdown_helpers.h"
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
	// 	ACTION_LEFT,
	// 	ACTION_RIGHT,
	// 	ACTION_RIGHT,

	// 	ACTION_DOWN,
	// 	ACTION_DOWN,
	// 	ACTION_DOWN,

	// 	ACTION_CLICK,
	// };

	vector<int> initial_solution{
		ACTION_DOWN,
		ACTION_LEFT,
		ACTION_UP,
		ACTION_RIGHT,
		ACTION_DOWN,
		ACTION_RIGHT,

		ACTION_DOWN,
		ACTION_LEFT,
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

	{
		Problem* problem = problem_type->get_problem();

		for (int a_index = 0; a_index < (int)initial_solution.size(); a_index++) {
			problem->perform_action(initial_solution[a_index]);
		}

		double result = problem->score_result();
		cout << "result: " << result << endl;

		delete problem;
	}

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

	vector<pair<int,int>> splits;
	init_breakdown(initial_solution,
				   lower_bounds,
				   upper_bounds,
				   splits);

	cout << "splits:" << endl;
	for (int s_index = 0; s_index < (int)splits.size(); s_index++) {
		cout << splits[s_index].first << " " << splits[s_index].second << endl;
	}

	delete problem_type;

	cout << "Done" << endl;
}
