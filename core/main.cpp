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
			Solution* duplicate = run_helper.success_duplicate;
			run_helper.success_duplicate = NULL;

			#if defined(MDEBUG) && MDEBUG
			while (duplicate->verify_problems.size() > 0) {
				Problem* problem = duplicate->verify_problems[0];

				RunHelper run_helper;
				run_helper.verify_key = duplicate->verify_key;

				run_helper.curr_run_seed = duplicate->verify_seeds[0];
				cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
				duplicate->verify_seeds.erase(duplicate->verify_seeds.begin());

				vector<ContextLayer> context;
				context.push_back(ContextLayer());

				context.back().scope = duplicate->scopes[this->id];
				context.back().node = NULL;

				ScopeHistory* root_history = new ScopeHistory(duplicate->scopes[this->id]);
				context.back().scope_history = root_history;

				duplicate->scopes[this->id]->verify_activate(
					problem,
					context,
					run_helper,
					root_history);

				delete root_history;

				delete duplicate->verify_problems[0];
				duplicate->verify_problems.erase(duplicate->verify_problems.begin());
			}
			duplicate->clear_verify();
			#endif /* MDEBUG */

			double sum_vals = 0.0;
			double sum_timestamp_score = 0.0;
			int timestamp_score_count = 0;
			double sum_instances_per_run = 0;
			double sum_local_num_actions = 0.0;
			for (int i_index = 0; i_index < MEASURE_ITERS; i_index++) {
				// Problem* problem = new Sorting();
				Problem* problem = new Minesweeper();

				RunHelper run_helper;
				Metrics metrics(solution->explore_id,
								solution->explore_type,
								duplicate->explore_id,
								duplicate->explore_type);

				vector<ContextLayer> context;
				context.push_back(ContextLayer());

				context.back().scope = duplicate->scopes[0];
				context.back().node = NULL;

				ScopeHistory* root_history = new ScopeHistory(duplicate->scopes[0]);
				context.back().scope_history = root_history;

				duplicate->scopes[0]->measure_activate(
					metrics,
					problem,
					context,
					run_helper,
					root_history);

				delete root_history;

				double target_val;
				if (run_helper.num_actions > duplicate->num_actions_limit) {
					target_val = -1.0;
				} else {
					target_val = problem->score_result(run_helper.num_decisions);
				}

				sum_vals += target_val;
				if (solution->explore_type == EXPLORE_TYPE_SCORE) {
					sum_timestamp_score += target_val;
					timestamp_score_count++;
				} else {
					for (int p_index = 0; p_index < (int)metrics.curr_predicted_scores.size(); p_index++) {
						double misguess = (target_val - metrics.curr_predicted_scores[p_index]) * (target_val - metrics.curr_predicted_scores[p_index]);
						sum_timestamp_score -= misguess;
						timestamp_score_count++;
					}
				}
				if (run_helper.num_actions > duplicate->max_num_actions) {
					duplicate->max_num_actions = run_helper.num_actions;
				}
				sum_instances_per_run += metrics.next_num_instances;
				if (metrics.next_max_num_actions > duplicate->explore_scope_max_num_actions) {
					duplicate->explore_scope_max_num_actions = metrics.next_max_num_actions;
				}
				sum_local_num_actions += metrics.next_local_sum_num_actions;

				delete problem;
			}
			duplicate->timestamp_score = sum_timestamp_score / timestamp_score_count;
			duplicate->curr_average_score = sum_vals / MEASURE_ITERS;
			duplicate->explore_average_instances_per_run = (double)sum_instances_per_run / (double)MEASURE_ITERS;
			duplicate->explore_scope_local_average_num_actions = sum_local_num_actions / sum_instances_per_run;

			#if defined(MDEBUG) && MDEBUG
			delete solution;
			solution = duplicate;

			solution->timestamp++;
			solution->save("", "main");

			ofstream display_file;
			display_file.open("../display.txt");
			solution->save_for_display(display_file);
			display_file.close();
			#else
			delete duplicate;
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
