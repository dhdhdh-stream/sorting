#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "run_helper.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index = 0;

int main(int argc, char* argv[]) {
	string filename;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->init();

	double sum_score = 0.0;
	double sum_true_score = 0.0;
	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(
			problem,
			run_helper,
			scope_history);
		delete scope_history;

		double target_val = problem->score_result();
		sum_score += target_val - 0.05 * run_helper.num_actions * solution->curr_time_penalty
			- run_helper.num_analyze * solution->curr_time_penalty;
		sum_true_score += target_val;

		delete problem;
	}

	solution->curr_score = sum_score / MEASURE_ITERS;
	solution->curr_true_score = sum_true_score / MEASURE_ITERS;

	solution->save("saves/", filename);

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
