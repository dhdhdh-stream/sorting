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

	RunHelper run_helper;

	{
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		uniform_int_distribution<int> retry_distribution(0, 1);
		run_helper.can_restart = retry_distribution(generator) == 0;

		cout << "run_helper.can_restart: " << run_helper.can_restart << endl;

		vector<ScopeHistory*> root_histories;
		run_helper.should_restart = true;
		while (run_helper.should_restart) {
			run_helper.curr_depth = 0;
			run_helper.throw_id = -1;
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

		cout << "root_histories.size(): " << root_histories.size() << endl;

		for (int h_index = 0; h_index < (int)root_histories.size(); h_index++) {
			delete root_histories[h_index];
		}

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
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
