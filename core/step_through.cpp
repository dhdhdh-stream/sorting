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

Problem* problem_type;
Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->load("", "main");

	{
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		// context.back().scope = solution->scopes[0];
		context.back().scope = solution->scopes[1];
		context.back().node = NULL;

		// ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		ScopeHistory* root_history = new ScopeHistory(solution->scopes[1]);
		context.back().scope_history = root_history;

		// solution->scopes[0]->step_through_activate(
		solution->scopes[1]->step_through_activate(
			problem,
			context,
			run_helper,
			root_history);

		string input_gate;
		cin >> input_gate;

		problem->print();
		cout << "Done" << endl;

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_decisions);
		} else {
			target_val = -1.0;
		}
		cout << "target_val: " << target_val << endl;

		delete root_history;

		delete problem;
	}

	cout << "Seed: " << seed << endl;

	delete solution;

	cout << "Done" << endl;
}
