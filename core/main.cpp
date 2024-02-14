#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_node.h"
#include "globals.h"
#include "minesweeper.h"
#include "outer_experiment.h"
#include "pass_through_experiment.h"
#include "simple.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "sorting.h"

using namespace std;

const int NUM_FAILS_BEFORE_INCREASE = 50;

default_random_engine generator;

Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	// solution->init();
	solution->load("", "main");

	// solution->save("", "main");

	int num_fails = 0;

	#if defined(MDEBUG) && MDEBUG
	int run_index = 0;
	#endif /* MDEBUG */

	while (true) {
		// Problem* problem = new Sorting();
		// Problem* problem = new Minesweeper();
		Problem* problem = new Simple();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = run_index;
		run_index++;
		#endif /* MDEBUG */

		bool outer_is_selected = false;
		if (solution->outer_experiment != NULL) {
			outer_is_selected = solution->outer_experiment->activate(
				problem,
				run_helper);
		}
		if (!outer_is_selected) {
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

			if (run_helper.experiment_history == NULL) {
				if (run_helper.experiments_seen_order.size() == 0) {
					#if defined(MDEBUG) && MDEBUG
					/**
					 * - allow exceeded_limit during debug to help test recursion
					 */
					create_experiment(root_history);
					#else
					if (!run_helper.exceeded_limit) {
						create_experiment(root_history);
					}
					#endif /* MDEBUG */
				}
			}

			delete root_history;
		}

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
		} else {
			target_val = -1.0;
		}

		bool is_success = false;
		bool is_fail = false;
		if (run_helper.experiment_history != NULL) {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
						+ run_helper.experiment_history->experiment->average_remaining_experiments_from_start);
			}

			run_helper.experiment_history->experiment->backprop(
				target_val,
				run_helper,
				run_helper.experiment_history);
			if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
				is_fail = true;
				run_helper.experiment_history->experiment->finalize();
				delete run_helper.experiment_history->experiment;
			} else if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				is_success = true;
				run_helper.experiment_history->experiment->finalize();
				delete run_helper.experiment_history->experiment;
			}
		} else {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index);
			}
		}

		delete problem;

		if (is_success) {
			solution->success_reset();

			#if defined(MDEBUG) && MDEBUG
			while (solution->verify_problems.size() > 0) {
				Problem* problem = solution->verify_problems[0];

				RunHelper run_helper;
				run_helper.verify_key = solution->verify_key;

				run_helper.starting_run_seed = solution->verify_seeds[0];
				run_helper.curr_run_seed = solution->verify_seeds[0];
				solution->verify_seeds.erase(solution->verify_seeds.begin());

				vector<ContextLayer> context;
				context.push_back(ContextLayer());

				context.back().scope = solution->root;
				context.back().node = NULL;

				ScopeHistory* root_history = new ScopeHistory(solution->root);
				context.back().scope_history = root_history;

				// unused
				int exit_depth = -1;
				AbstractNode* exit_node = NULL;

				solution->root->verify_activate(problem,
												context,
												exit_depth,
												exit_node,
												run_helper,
												root_history);

				delete root_history;

				delete solution->verify_problems[0];
				solution->verify_problems.erase(solution->verify_problems.begin());
			}
			solution->clear_verify();
			#endif /* MDEBUG */

			num_fails = 0;

			// solution->timestamp = (unsigned)time(NULL);
			// solution->save("", "main");

			// ofstream display_file;
			// display_file.open("../display.txt");
			// solution->save_for_display(display_file);
			// display_file.close();

			#if defined(MDEBUG) && MDEBUG
			solution->depth_limit = solution->max_depth + 1;
			#else
			if (solution->max_depth < 50) {
				solution->depth_limit = solution->max_depth + 10;
			} else {
				solution->depth_limit = (int)(1.2*(double)solution->max_depth);
			}
			#endif /* MDEBUG */

			solution->curr_num_datapoints = STARTING_NUM_DATAPOINTS;
		} else if (is_fail) {
			num_fails++;
			cout << "num_fails: " << num_fails << endl << endl;
			if (num_fails > NUM_FAILS_BEFORE_INCREASE) {
				cout << "fail_reset" << endl << endl;

				num_fails = 0;
				solution->fail_reset();

				solution->curr_num_datapoints *= 2;
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (run_index%1000 == 0) {
			delete solution;
			solution = new Solution();
			solution->load("", "main");
		}
		#endif /* MDEBUG */
	}

	delete solution;

	cout << "Done" << endl;
}
