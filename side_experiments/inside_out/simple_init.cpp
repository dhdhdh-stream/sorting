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
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

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

	ProblemType* problem_type = new TypeMinesweeper();

	SolutionWrapper* solution_wrapper = new SolutionWrapper(
		problem_type->num_obs());

	double sum_score = 0.0;
	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		solution_wrapper->measure_init();

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = solution_wrapper->measure_step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * solution_wrapper->num_actions;
		sum_score += target_val;

		solution_wrapper->measure_end(target_val);

		delete problem;
	}

	double new_score = sum_score / (double)MEASURE_ITERS;
	cout << "new_score: " << new_score << endl;
	solution_wrapper->measure_update(new_score);

	solution_wrapper->save("saves/", filename);

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
