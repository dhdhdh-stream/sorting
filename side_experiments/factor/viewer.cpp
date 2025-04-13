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

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int multi_index = 0;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
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

		double target_val = problem->score_result();
		target_val -= run_helper.num_actions * solution->curr_time_penalty;
		cout << "target_val: " << target_val << endl;

		problem->print();

		cout << "run_helper.num_actions: " << run_helper.num_actions << endl;

		delete problem;
	}

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
