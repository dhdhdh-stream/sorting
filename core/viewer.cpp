#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "eval.h"
#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
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

	// problem_type = new Sorting();
	problem_type = new Minesweeper();

	solution = new Solution();
	solution->load("", "main");

	// temp
	solution->state = SOLUTION_STATE_EVAL;
	uniform_int_distribution<int> next_distribution(0, (int)(2.0 * solution->average_num_actions));
	solution->num_actions_until_random = 1 + next_distribution(generator);

	{
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = solution->scopes[0];
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		context.back().scope_history = root_history;

		solution->scopes[0]->activate(
			problem,
			context,
			run_helper,
			root_history);

		problem->print();

		double predicted_score;
		if (solution->state == SOLUTION_STATE_EVAL) {
			predicted_score = solution->eval->activate(problem,
													   run_helper);
			cout << "predicted_score: " << predicted_score << endl;
		}

		delete root_history;

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_decisions);
		} else {
			target_val = -1.0;
		}
		cout << "target_val: " << target_val << endl;

		problem->print();

		cout << "run_helper.num_actions: " << run_helper.num_actions << endl;
		cout << "run_helper.num_decisions: " << run_helper.num_decisions << endl;

		delete problem;
	}

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete solution;

	cout << "Done" << endl;
}
