#include <chrono>
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

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

const int BRANCH_FACTOR = 4;

int main(int argc, char* argv[]) {
	if (argc != 1 + BRANCH_FACTOR + 1) {
		cout << "Usage: ./combine [child ... files] [output]" << endl;
		exit(1);
	}
	
	vector<string> child_files(BRANCH_FACTOR);
	for (int c_index = 0; c_index < BRANCH_FACTOR; c_index++) {
		child_files[c_index] = argv[1 + c_index];
	}
	string output_file = argv[1 + BRANCH_FACTOR];

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->init();

	for (int existing_index = 0; existing_index < BRANCH_FACTOR; existing_index++) {
		Solution* existing_solution = new Solution();
		existing_solution->load("saves/", child_files[existing_index]);

		for (int scope_index = 1; scope_index < (int)existing_solution->scopes.size(); scope_index++) {
			solution->scopes.push_back(existing_solution->scopes[scope_index]);
		}

		existing_solution->scopes.erase(existing_solution->scopes.begin() + 1, existing_solution->scopes.end());

		delete existing_solution;
	}

	for (int scope_index = 1; scope_index < (int)solution->scopes.size(); scope_index++) {
		solution->scopes[scope_index]->id = scope_index;
	}

	for (int scope_index = 1; scope_index < (int)solution->scopes.size(); scope_index++) {
		solution->scopes[0]->existing_scopes.push_back(solution->scopes[scope_index]);
	}

	solution->num_existing_scopes = (int)solution->scopes.size() - 1;

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

	commit_helper();

	solution->save("saves/", output_file);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
