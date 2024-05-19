// TODO: add mimicking
// - human performs simple full run, so no decision making/variations
//   - save obs
//   - copy sequences, and try using them in different places
//   - or examine differences/branching offs
// - need to be able to go from action sequence to matching scope
// - maybe need multiple runs?
//   - really not that much to do with a single run
//     - perform exactly and hope good enough
//     - but can easily fail because solution is sharp and doesn't have the fallback the original person has
// - yeah, so have multiple runs, which shouldn't be that big of a problem
//   - hopefully, representative ones
// - initially, runs will look like a tree with branch really close to root
//   - need to assume that there's repetition and try to change into a graph?
//     - is it even worth to spend effort here?
//       - probably, at the very least to transfer knowledge from humans

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "eval.h"
#include "globals.h"
#include "minesweeper.h"
#include "sorting.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "sorting.h"
#include "utilities.h"

using namespace std;

int seed;

default_random_engine generator;

Problem* problem_type;
Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// problem_type = new Sorting();
	problem_type = new Minesweeper();

	solution = new Solution();
	// solution->init();
	solution->load("", "main");

	// solution->save("", "main");

	#if defined(MDEBUG) && MDEBUG
	int run_index = 0;
	#endif /* MDEBUG */

	while (true) {
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = run_index;
		run_index++;
		#endif /* MDEBUG */

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = solution->scopes[0];
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		context.back().scope_history = root_history;

		solution->scopes[0]->activate(
			problem,
			context,
			run_helper,
			root_history);

		delete root_history;

		delete problem;

		if (run_helper.success_duplicate != NULL) {
			#if defined(MDEBUG) && MDEBUG
			while (run_helper.success_duplicate->verify_problems.size() > 0) {
				Problem* problem = run_helper.success_duplicate->verify_problems[0];

				RunHelper run_helper;
				run_helper.verify_key = run_helper.success_duplicate->verify_key;

				run_helper.curr_run_seed = run_helper.success_duplicate->verify_seeds[0];
				cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
				run_helper.success_duplicate->verify_seeds.erase(run_helper.success_duplicate->verify_seeds.begin());

				vector<ContextLayer> context;
				context.push_back(ContextLayer());

				context.back().scope = run_helper.success_duplicate->scopes[this->id];
				context.back().node = NULL;

				ScopeHistory* root_history = new ScopeHistory(run_helper.success_duplicate->scopes[this->id]);
				context.back().scope_history = root_history;

				run_helper.success_duplicate->scopes[this->id]->verify_activate(
					problem,
					context,
					run_helper,
					root_history);

				delete root_history;

				delete run_helper.success_duplicate->verify_problems[0];
				run_helper.success_duplicate->verify_problems.erase(run_helper.success_duplicate->verify_problems.begin());
			}
			run_helper.success_duplicate->clear_verify();

			run_helper.success_duplicate->increment();

			run_helper.success_duplicate->average_num_actions = run_helper.success_duplicate->max_num_actions;

			delete solution;
			solution = run_helper.success_duplicate;

			run_helper.success_duplicate->timestamp++;
			run_helper.success_duplicate->save("", "main");

			ofstream display_file;
			display_file.open("../display.txt");
			run_helper.success_duplicate->save_for_display(display_file);
			display_file.close();
			#else
			delete run_helper.success_duplicate;
			#endif /* MDEBUG */
		}

		#if defined(MDEBUG) && MDEBUG
		if (run_index%5000 == 0) {
			delete solution;
			solution = new Solution();
			solution->load("", "main");
		}
		#endif /* MDEBUG */
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
