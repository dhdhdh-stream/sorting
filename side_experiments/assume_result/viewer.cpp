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

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->load("", "main");

	cout << "solution->curr_true_score: " << solution->curr_true_score << endl;
	cout << "solution->best_true_score: " << solution->best_true_score << endl;
	cout << "solution->best_true_score_timestamp: " << solution->best_true_score_timestamp << endl;
	cout << "solution->curr_time_penalty: " << solution->curr_time_penalty << endl;

	{
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		vector<ContextLayer> context;
		solution->scopes[0]->activate(
			problem,
			context,
			run_helper);

		double target_val;
		if (run_helper.exceeded_limit) {
			target_val = -1.0;
		} else {
			target_val = problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;
		}
		cout << "target_val: " << target_val << endl;

		problem->print();

		cout << "run_helper.num_analyze: " << run_helper.num_analyze << endl;
		cout << "run_helper.num_actions: " << run_helper.num_actions << endl;

		delete problem;
	}

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete solution;

	cout << "Done" << endl;
}
