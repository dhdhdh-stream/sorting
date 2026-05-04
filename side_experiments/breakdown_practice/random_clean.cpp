// - random is OK

// - not necessarily even better to clean fully?

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

	vector<int> curr_solution{
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

	while (true) {
		vector<int> removable;
		for (int a_index = 0; a_index < (int)curr_solution.size(); a_index++) {
			vector<int> test_solution = curr_solution;
			test_solution.erase(test_solution.begin() + a_index);

			Problem* problem = problem_type->get_problem();

			for (int i_index = 0; i_index < (int)test_solution.size(); i_index++) {
				problem->perform_action(test_solution[i_index]);
			}

			double result = problem->score_result();

			delete problem;

			if (result == 1.0) {
				removable.push_back(a_index);
			}
		}

		if (removable.size() == 0) {
			break;
		} else {
			// temp
			cout << "removable.size(): " << removable.size() << endl;

			uniform_int_distribution<int> distribution(0, removable.size()-1);
			int index = distribution(generator);
			curr_solution.erase(curr_solution.begin() + removable[index]);
		}
	}

	for (int a_index = 0; a_index < (int)curr_solution.size(); a_index++) {
		cout << curr_solution[a_index] << " ";
	}
	cout << endl;

	delete problem_type;

	cout << "Done" << endl;
}
