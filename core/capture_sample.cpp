#include <iostream>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "sample.h"
#include "solution_set.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
SolutionSet* solution_set;

#if defined(MDEBUG) && MDEBUG
int run_index = 0;
#endif /* MDEBUG */

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// problem_type = new TypeSorting();
	problem_type = new TypeMinesweeper();

	Problem* problem = problem_type->get_problem();

	Sample sample;
	sample.id = 0;

	sample.actions.push_back(Action(ACTION_NOOP));

	while (true) {
		vector<double> curr_obs = problem->get_observations();
		sample.obs.push_back(curr_obs);

		problem->print_obs();

		cout << "input:" << endl;

		string input;
		cin >> input;

		if (input.compare("x") == 0) {
			break;
		} else {
			int move = stoi(input);
			sample.actions.push_back(Action(move));
			problem->perform_action(Action(move));
		}
	}

	sample.save();

	cout << "Done" << endl;
}
