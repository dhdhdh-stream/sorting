#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	if (argc > 1) {
		string filename = argv[1];
		solution->load("saves/", filename);
	} else {
		solution->load("saves/", "main.txt");
	}

	{
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(
			problem,
			run_helper,
			scope_history);
		delete scope_history;

		double target_val = run_helper.sum_score + problem->score_result();
		cout << "target_val: " << target_val << endl;

		problem->print();

		cout << "run_helper.num_actions: " << run_helper.num_actions << endl;

		delete problem;
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
