#include <iostream>

#include "globals.h"
#include "simple_problem.h"

using namespace std;

default_random_engine generator;

ProblemType* problem_type;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeSimpleProblem();

	int num_zero = 0;
	int num_good = 0;

	uniform_int_distribution<int> length_distribution(1, 49);
	uniform_int_distribution<int> action_distribution(0, 2);
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		Problem* problem = problem_type->get_problem();

		int length = length_distribution(generator);

		for (int l_index = 0; l_index < length; l_index++) {
			int move = action_distribution(generator);
			problem->perform_action(Action(move));
		}

		double target_val = problem->score_result();
		if (target_val == 0.0) {
			num_zero++;
		}
		if (target_val == 1.0) {
			num_good++;
		}

		delete problem;
	}

	cout << "num_zero: " << num_zero << endl;
	cout << "num_good: " << num_good << endl;

	delete problem_type;

	cout << "Done" << endl;
}
