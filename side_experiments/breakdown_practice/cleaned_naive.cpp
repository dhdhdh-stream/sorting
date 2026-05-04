// TODO: split until everything within each section can move around freely
// - but want the minimum number of splits

// TODO: multi-level breakdown

// TODO: create scopes based on breakdown (rather than new scopes)

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

	vector<int> initial_solution{
		ACTION_LEFT,
		ACTION_RIGHT,
		ACTION_RIGHT,

		ACTION_DOWN,
		ACTION_DOWN,
		ACTION_DOWN,

		ACTION_CLICK,
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

	vector<int> split_scores(initial_solution.size(), 0);
	for (int a_index = 0; a_index < (int)lower_bounds.size(); a_index++) {
		if (lower_bounds[a_index] > 0) {
			split_scores[lower_bounds[a_index]]++;
		}
	}
	for (int a_index = 0; a_index < (int)upper_bounds.size(); a_index++) {
		if (upper_bounds[a_index] < (int)initial_solution.size()-1) {
			split_scores[upper_bounds[a_index]]++;
		}
	}

	cout << "split_scores:" << endl;
	for (int a_index = 0; a_index < (int)split_scores.size(); a_index++) {
		cout << a_index << ": " << split_scores[a_index] << endl;
	}

	delete problem_type;

	cout << "Done" << endl;
}
