#include <iostream>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "sorting.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

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

	solution = new Solution();
	solution->load("", "main");

	{
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		vector<ContextLayer> context;
		solution->scopes[0]->step_through_activate(
			problem,
			context,
			run_helper);

		string input_gate;
		cin >> input_gate;

		problem->print();
		cout << "Done" << endl;

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = -1.0;
		} else {
			target_val = problem->score_result(run_helper.num_decisions);
		}
		cout << "target_val: " << target_val << endl;

		delete problem;
	}

	cout << "Seed: " << seed << endl;

	delete solution;

	cout << "Done" << endl;
}
