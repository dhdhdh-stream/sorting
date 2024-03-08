#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"
#include "sorting.h"

using namespace std;

default_random_engine generator;

Problem* problem_type;
Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->load("", "main");

	double sum_vals = 0.0;

	for (int i_index = 0; i_index < 2000; i_index++) {
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		uniform_int_distribution<int> retry_distribution(0, 1);
		run_helper.can_restart = retry_distribution(generator) == 0;

		vector<ScopeHistory*> root_histories;
		run_helper.should_restart = true;
		while (run_helper.should_restart) {
			run_helper.should_restart = false;

			vector<ContextLayer> context;
			context.push_back(ContextLayer());

			context.back().scope = solution->root;
			context.back().node = NULL;

			ScopeHistory* root_history = new ScopeHistory(solution->root);
			context.back().scope_history = root_history;

			// unused
			int exit_depth = -1;
			AbstractNode* exit_node = NULL;

			solution->root->activate(problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper,
									 root_history);

			root_histories.push_back(root_history);
		}

		for (int h_index = 0; h_index < (int)root_histories.size(); h_index++) {
			delete root_histories[h_index];
		}

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
		} else {
			target_val = -1.0;
		}
		sum_vals += target_val;

		delete problem;
	}

	cout << "average score: " << sum_vals/2000 << endl;

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete solution;

	cout << "Done" << endl;
}
