#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_set.h"
#include "sorting.h"

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

	solution_set = new SolutionSet();
	solution_set->load("", "main");

	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];

	vector<AbstractScopeHistory*> scope_histories;
	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		vector<ContextLayer> context;
		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(
			problem,
			context,
			run_helper,
			scope_history);
		scope_histories.push_back(scope_history);

		delete problem;
	}

	add_eval_familiarity(solution->scopes[0],
						 scope_histories);
	for (int d_index = 0; d_index < (int)scope_histories.size(); d_index++) {
		delete scope_histories[d_index];
	}

	// temp
	{
		vector<AbstractScopeHistory*> scope_histories;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			vector<ContextLayer> context;
			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->activate(
				problem,
				context,
				run_helper,
				scope_history);
			scope_histories.push_back(scope_history);

			delete problem;
		}

		measure_familiarity(solution->scopes[0],
							scope_histories);
		for (int d_index = 0; d_index < (int)scope_histories.size(); d_index++) {
			delete scope_histories[d_index];
		}
	}

	solution_set->save("", "familiarity");

	delete solution;

	cout << "Done" << endl;
}
