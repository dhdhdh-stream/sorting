#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "focus_minesweeper.h"
#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"
#include "solution_set.h"
#include "sorting.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
SolutionSet* solution_set;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// problem_type = new TypeSorting();
	// problem_type = new TypeMinesweeper();
	problem_type = new TypeFocusMinesweeper();

	solution_set = new SolutionSet();
	solution_set->load("", "main");

	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];

	double sum_vals = 0.0;
	int max_num_actions = 0;

	auto start_time = chrono::high_resolution_clock::now();
	for (int i_index = 0; i_index < 2000; i_index++) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		vector<ContextLayer> context;
		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(
			problem,
			context,
			run_helper,
			scope_history);
		delete scope_history;

		double target_val;
		if (run_helper.exceeded_limit) {
			target_val = -1.0;
		} else {
			target_val = problem->score_result(run_helper.num_decisions,
											   run_helper.num_actions);
		}

		if (run_helper.num_actions > max_num_actions) {
			max_num_actions = run_helper.num_actions;
		}

		sum_vals += target_val;

		delete problem;
	}

	cout << "average score: " << sum_vals/2000 << endl;
	cout << "max_num_actions: " << max_num_actions << endl;

	auto curr_time = chrono::high_resolution_clock::now();
	auto time_diff = chrono::duration_cast<chrono::milliseconds>(curr_time - start_time);
	cout << "time_diff.count(): " << time_diff.count() << endl;

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete solution_set;

	cout << "Done" << endl;
}
