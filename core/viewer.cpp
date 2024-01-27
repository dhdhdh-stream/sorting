#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "context_layer.h"
#include "globals.h"
#include "solution_helpers.h"
#include "minesweeper.h"
#include "run_helper.h"
#include "scope.h"
#include "solution.h"
#include "sorting.h"
#include "state_status.h"

using namespace std;

default_random_engine generator;

Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->load("", "main");

	cout << "solution->states.size(): " << solution->states.size() << endl;

	{
		Problem* problem = new Sorting();
		// Problem* problem = new Minesweeper();

		RunHelper run_helper;

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = solution->root;
		context.back().node = NULL;

		// unused
		int exit_depth = -1;
		AbstractNode* exit_node = NULL;

		solution->root->view_activate(problem,
									  context,
									  exit_depth,
									  exit_node,
									  run_helper);

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_process);
		} else {
			target_val = -1.0;
		}
		cout << "target_val: " << target_val << endl;

		problem->print();

		delete problem;
	}

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete solution;

	cout << "Done" << endl;
}
