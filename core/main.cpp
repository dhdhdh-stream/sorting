// TODO: add mimicking
// - human performs simple full run, so no decision making/variations
//   - copy sequences, and try using them in different places
//   - or examine differences/branching offs
// - need to be able to go from action sequence to matching scope

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "globals.h"
#include "minesweeper.h"
#include "new_action_tracker.h"
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

		context.back().scope = solution->current;
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(solution->current);
		context.back().scope_history = root_history;

		solution->current->activate(
			problem,
			context,
			run_helper,
			root_history);

		if (solution->state == SOLUTION_STATE_TRAVERSE) {
			if (run_helper.experiments_seen_order.size() == 0) {
				if (!run_helper.exceeded_limit) {
					create_experiment(root_history);
				}
			}
		}

		delete root_history;

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_decisions);
		} else {
			target_val = -1.0;
		}

		if (run_helper.experiment_histories.size() > 0) {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
						+ run_helper.experiment_histories[0]->experiment->average_remaining_experiments_from_start);
			}
			for (int h_index = 0; h_index < (int)run_helper.experiment_histories.size()-1; h_index++) {
				AbstractExperimentHistory* experiment_history = run_helper.experiment_histories[h_index];
				for (int e_index = 0; e_index < (int)experiment_history->experiments_seen_order.size(); e_index++) {
					AbstractExperiment* experiment = experiment_history->experiments_seen_order[e_index];
					experiment->average_remaining_experiments_from_start =
						0.9 * experiment->average_remaining_experiments_from_start
						+ 0.1 * ((int)experiment_history->experiments_seen_order.size()-1 - e_index
							+ run_helper.experiment_histories[h_index+1]->experiment->average_remaining_experiments_from_start);
				}
			}
			{
				/**
				 * - non-empty if EXPERIMENT_STATE_EXPERIMENT
				 */
				AbstractExperimentHistory* experiment_history = run_helper.experiment_histories.back();
				for (int e_index = 0; e_index < (int)experiment_history->experiments_seen_order.size(); e_index++) {
					AbstractExperiment* experiment = experiment_history->experiments_seen_order[e_index];
					experiment->average_remaining_experiments_from_start =
						0.9 * experiment->average_remaining_experiments_from_start
						+ 0.1 * ((int)experiment_history->experiments_seen_order.size()-1 - e_index);
				}
			}

			run_helper.experiment_histories.back()->experiment->backprop(
				target_val,
				run_helper);
			if (run_helper.experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_FAIL) {
				if (run_helper.experiment_histories.size() == 1) {
					run_helper.experiment_histories.back()->experiment->finalize(NULL);
					delete run_helper.experiment_histories.back()->experiment;
				} else {
					AbstractExperiment* curr_experiment = run_helper.experiment_histories.back()->experiment->parent_experiment;

					curr_experiment->experiment_iter++;
					int matching_index;
					for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
						if (curr_experiment->child_experiments[c_index] == run_helper.experiment_histories.back()->experiment) {
							matching_index = c_index;
							break;
						}
					}
					curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

					run_helper.experiment_histories.back()->experiment->result = EXPERIMENT_RESULT_FAIL;
					run_helper.experiment_histories.back()->experiment->finalize(NULL);
					delete run_helper.experiment_histories.back()->experiment;

					double target_count = (double)MAX_EXPERIMENT_NUM_EXPERIMENTS
						* pow(0.5, run_helper.experiment_histories.size()-1);
					while (true) {
						if (curr_experiment == NULL) {
							break;
						}

						if (curr_experiment->experiment_iter >= target_count) {
							AbstractExperiment* parent = curr_experiment->parent_experiment;

							if (parent != NULL) {
								parent->experiment_iter++;
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
							curr_experiment->finalize(NULL);
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
				Solution* duplicate = new Solution(solution);
				run_helper.experiment_histories.back()->experiment->finalize(duplicate);
				delete run_helper.experiment_histories.back()->experiment;

				#if defined(MDEBUG) && MDEBUG
				while (duplicate->verify_problems.size() > 0) {
					Problem* problem = duplicate->verify_problems[0];

					RunHelper run_helper;
					run_helper.verify_key = duplicate->verify_key;

					run_helper.starting_run_seed = duplicate->verify_seeds[0];
					cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
					run_helper.curr_run_seed = duplicate->verify_seeds[0];
					duplicate->verify_seeds.erase(duplicate->verify_seeds.begin());

					vector<ContextLayer> context;
					context.push_back(ContextLayer());

					context.back().scope = duplicate->current;
					context.back().node = NULL;

					ScopeHistory* root_history = new ScopeHistory(duplicate->current);
					context.back().scope_history = root_history;

					duplicate->current->verify_activate(
						problem,
						context,
						run_helper,
						root_history);

					delete root_history;

					delete duplicate->verify_problems[0];
					duplicate->verify_problems.erase(duplicate->verify_problems.begin());
				}
				duplicate->clear_verify();

				duplicate->increment();

				duplicate->average_num_actions = duplicate->max_num_actions;

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
		} else {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index);
			}

			if (run_helper.new_action_history != NULL) {
				for (int n_index = 0; n_index < (int)run_helper.new_action_history->existing_path_taken.size(); n_index++) {
					NewActionNodeTracker* node_tracker = solution->new_action_tracker->node_trackers[
						run_helper.new_action_history->existing_path_taken[n_index]];
					node_tracker->existing_score += target_val;
					node_tracker->existing_count++;
				}
				for (int n_index = 0; n_index < (int)run_helper.new_action_history->new_path_taken.size(); n_index++) {
					NewActionNodeTracker* node_tracker = solution->new_action_tracker->node_trackers[
						run_helper.new_action_history->new_path_taken[n_index]];
					node_tracker->new_score += target_val;
					node_tracker->new_count++;
				}
			}
		}

		delete problem;

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
