#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

const int TRIM_TRIES = 50;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->load("", "main");

	solution->save("", "checkpoint");

	Solution* best_trim = NULL;
	double best_score;
	for (int t_index = 0; t_index < TRIM_TRIES; t_index++) {
		Solution* duplicate = new Solution(solution);
		duplicate->random_trim();

		double sum_score = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			vector<ContextLayer> context;
			duplicate->scopes[0]->measure_activate(
				problem,
				context,
				run_helper);

			double target_val;
			if (!run_helper.exceeded_limit) {
				target_val = problem->score_result(run_helper.num_analyze,
												   run_helper.num_actions);
			} else {
				target_val = -1.0;
			}

			sum_score += target_val;

			delete problem;
		}

		double average_score = sum_score / MEASURE_ITERS;

		if (best_trim == NULL
				|| average_score > best_score) {
			if (best_trim != NULL) {
				delete best_trim;
			}
			best_trim = duplicate;
			best_score = average_score;
		}
	}

	best_trim->save("", "main");

	delete best_trim;

	delete solution;

	cout << "Done" << endl;
}
