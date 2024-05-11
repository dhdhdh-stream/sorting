#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "eval.h"
#include "globals.h"
#include "minesweeper.h"
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

	// problem_type = new Sorting();
	problem_type = new Minesweeper();

	solution = new Solution();
	solution->load("", "main");

	double sum_vals = 0.0;

	// temp
	solution->state = SOLUTION_STATE_EVAL;
	uniform_int_distribution<int> next_distribution(0, (int)(2.0 * solution->average_num_actions));
	solution->num_actions_until_random = 1 + next_distribution(generator);

	auto start_time = chrono::high_resolution_clock::now();
	for (int i_index = 0; i_index < 2000; i_index++) {
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = solution->current;
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(solution->current);
		context.back().scope_history = root_history;

		solution->current->activate(
			problem,
			context,
			run_helper,
			root_history);

		double predicted_score;
		if (solution->state == SOLUTION_STATE_EVAL) {
			predicted_score = solution->eval->activate(problem,
													   run_helper);
		}

		delete root_history;

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_decisions);
		} else {
			target_val = -1.0;
		}

		if (solution->state == SOLUTION_STATE_TRAVERSE) {
			sum_vals += target_val;
		} else if (solution->state == SOLUTION_STATE_EVAL) {
			double misguess = (target_val - predicted_score) * (target_val - predicted_score);
			sum_vals += -misguess;
		}

		delete problem;
	}

	cout << "average score: " << sum_vals/2000 << endl;

	auto curr_time = chrono::high_resolution_clock::now();
	auto time_diff = chrono::duration_cast<chrono::milliseconds>(curr_time - start_time);
	cout << "time_diff.count(): " << time_diff.count() << endl;

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete solution;

	cout << "Done" << endl;
}
