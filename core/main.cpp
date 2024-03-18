#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_node.h"
#include "branch_experiment.h"
#include "globals.h"
#include "minesweeper.h"
#include "pass_through_experiment.h"
#include "sorting.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "sorting.h"
#include "utilities.h"

using namespace std;

const int NUM_FAILS_BEFORE_INCREASE = 50;

default_random_engine generator;

Problem* problem_type;
Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// problem_type = new Sorting();
	problem_type = new Minesweeper();

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
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = run_index;
		run_index++;
		#endif /* MDEBUG */

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = solution->root;
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(solution->root);
		context.back().scope_history = root_history;

		// unused
		int exit_depth = -1;
		AbstractNode* exit_node = NULL;

		solution->root->activate(solution->root->default_starting_node,
								 problem,
								 context,
								 exit_depth,
								 exit_node,
								 run_helper,
								 root_history);

		if (run_helper.experiments_seen_order.size() == 0) {
			if (!run_helper.exceeded_limit) {
				create_experiment(root_history);
			}
		}

		delete root_history;

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
		} else {
			target_val = -1.0;
		}

		bool is_success = false;
		bool is_fail = false;
		if (run_helper.experiment_histories.size() > 0) {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
						+ run_helper.experiment_histories[0]->experiment->average_remaining_experiments_from_start);
			}
			for (int h_index = 0; h_index < (int)run_helper.experiment_histories.size()-1; h_index++) {
				PassThroughExperimentHistory* pass_through_experiment_history = (PassThroughExperimentHistory*)run_helper.experiment_histories[h_index];
				for (int e_index = 0; e_index < (int)pass_through_experiment_history->experiments_seen_order.size(); e_index++) {
					AbstractExperiment* experiment = pass_through_experiment_history->experiments_seen_order[e_index];
					experiment->average_remaining_experiments_from_start =
						0.9 * experiment->average_remaining_experiments_from_start
						+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
							+ run_helper.experiment_histories[h_index+1]->experiment->average_remaining_experiments_from_start);
				}
			}

			run_helper.experiment_histories.back()->experiment->backprop(
				target_val,
				run_helper);
			if (run_helper.experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_FAIL) {
				if (run_helper.experiment_histories.size() == 1) {
					is_fail = true;
					run_helper.experiment_histories.back()->experiment->finalize();
					delete run_helper.experiment_histories.back()->experiment;
				} else {
					PassThroughExperiment* curr_experiment = run_helper.experiment_histories.back()->experiment->parent_experiment;

					curr_experiment->state_iter++;
					int matching_index;
					for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
						if (curr_experiment->child_experiments[c_index] == run_helper.experiment_histories.back()->experiment) {
							matching_index = c_index;
							break;
						}
					}
					curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

					run_helper.experiment_histories.back()->experiment->result = EXPERIMENT_RESULT_FAIL;
					run_helper.experiment_histories.back()->experiment->finalize();
					delete run_helper.experiment_histories.back()->experiment;

					double target_count = (double)MAX_PASS_THROUGH_EXPERIMENT_NUM_EXPERIMENTS
						* pow(0.5, run_helper.experiment_histories.size()-1);
					while (true) {
						if (curr_experiment == NULL) {
							is_fail = true;
							break;
						}

						if (curr_experiment->state_iter >= target_count) {
							PassThroughExperiment* parent = curr_experiment->parent_experiment;

							if (parent != NULL) {
								parent->state_iter++;
								int matching_index;
								for (int c_index = 0; c_index < (int)parent->child_experiments.size(); c_index++) {
									if (parent->child_experiments[c_index] == curr_experiment) {
										matching_index = c_index;
										break;
									}
								}
								parent->child_experiments.erase(parent->child_experiments.begin() + matching_index);
							}

							curr_experiment->result = EXPERIMENT_RESULT_FAIL;
							curr_experiment->finalize();
							delete curr_experiment;

							curr_experiment = parent;
							target_count *= 2.0;
						} else {
							break;
						}
					}
				}
			} else if (run_helper.experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				/**
				 * - run_helper.experiment_histories.size() == 1
				 */
				is_success = true;
				run_helper.experiment_histories.back()->experiment->finalize();
				delete run_helper.experiment_histories.back()->experiment;
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

				solution->root->verify_activate(solution->root->default_starting_node,
												problem,
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

			#if defined(MDEBUG) && MDEBUG
			solution->depth_limit = solution->max_depth + 1;
			#else
			if (solution->max_depth < 50) {
				solution->depth_limit = solution->max_depth + 10;
			} else {
				solution->depth_limit = (int)(1.2*(double)solution->max_depth);
			}
			#endif /* MDEBUG */

			solution->num_actions_limit = 20*solution->max_num_actions + 20;

			// solution->timestamp = (unsigned)time(NULL);
			// solution->save("", "main");

			// ofstream display_file;
			// display_file.open("../display.txt");
			// solution->save_for_display(display_file);
			// display_file.close();

			num_fails = 0;

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
