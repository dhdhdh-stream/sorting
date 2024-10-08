/**
 * - humans teach incrementally
 *   - start by sharing most common sequence along with when to apply in general
 *     - so initial sequence may not perfectly match when to apply
 *   - then gradually add branches sequence by sequence along with when those apply
 * 
 * - requires shared actions/vocabulary
 * 
 * - so in order to learn from humans:
 *   - would need to learn actions/vocabulary
 *     - so would need to learn all the sequences/branches and decision making for those actions
 *       - and repeat recursively
 * 
 * TODO: add mimicking using world modeling/optical flow
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "constants.h"
#include "eval_helpers.h"
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

ProblemType* problem_type;
Solution* solution;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// problem_type = new TypeSorting();
	problem_type = new TypeMinesweeper();

	solution = new Solution();
	// solution->init();
	solution->load("", "main");

	update_eval();

	// solution->save("", "main");

	run_index = 0;

	while (true) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = run_index;
		#endif /* MDEBUG */
		run_index++;
		if (run_index%10000 == 0) {
			cout << "run_index: " << run_index << endl;
		}

		vector<ContextLayer> context;
		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(
			problem,
			context,
			run_helper,
			scope_history);
		delete scope_history;

		if (run_helper.experiments_seen_order.size() == 0) {
			if (!run_helper.exceeded_limit) {
				create_experiment(run_helper);
			}
		}

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_decisions,
											   run_helper.num_actions);
		} else {
			target_val = -1.0;
		}

		delete problem;

		#if defined(MDEBUG) && MDEBUG
		if (run_index%2000 == 0) {
			delete solution;
			solution = new Solution();
			solution->load("", "main");

			update_eval();

			continue;
		}
		#endif /* MDEBUG */

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
				 * - history->experiment_histories.size() == 1
				 */
				Solution* duplicate = new Solution(solution);

				int last_updated_info_scope_id = -1;
				if (run_helper.experiment_histories.back()->experiment->type == EXPERIMENT_TYPE_INFO_PASS_THROUGH) {
					last_updated_info_scope_id = run_helper.experiment_histories.back()->experiment->scope_context->id;
				} else {
					duplicate->last_updated_scope_id = run_helper.experiment_histories.back()->experiment->scope_context->id;
					if (run_helper.experiment_histories.back()->experiment->type == EXPERIMENT_TYPE_NEW_ACTION) {
						duplicate->last_new_scope_id = (int)duplicate->scopes.size();
					}
				}

				run_helper.experiment_histories.back()->experiment->finalize(duplicate);
				delete run_helper.experiment_histories.back()->experiment;

				if (last_updated_info_scope_id != -1) {
					InfoScope* info_scope = duplicate->info_scopes[last_updated_info_scope_id];
					clean_info_scope(info_scope);
				} else {
					Scope* experiment_scope = duplicate->scopes[duplicate->last_updated_scope_id];
					clean_scope(experiment_scope,
								duplicate);
				}

				#if defined(MDEBUG) && MDEBUG
				while (duplicate->verify_problems.size() > 0) {
					Problem* problem = duplicate->verify_problems[0];

					RunHelper run_helper;
					run_helper.starting_run_seed = duplicate->verify_seeds[0];
					cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
					run_helper.curr_run_seed = duplicate->verify_seeds[0];
					duplicate->verify_seeds.erase(duplicate->verify_seeds.begin());

					vector<ContextLayer> context;
					ScopeHistory* scope_history = new ScopeHistory(duplicate->scopes[0]);
					duplicate->scopes[0]->verify_activate(
						problem,
						context,
						run_helper,
						scope_history);
					delete scope_history;

					cout << "run_helper.num_actions: " << run_helper.num_actions << endl;
					cout << "duplicate->num_actions_limit: " << duplicate->num_actions_limit << endl;

					delete duplicate->verify_problems[0];
					duplicate->verify_problems.erase(duplicate->verify_problems.begin());
				}
				duplicate->clear_verify();
				#endif /* MDEBUG */

				vector<double> target_vals;
				int max_num_actions = 0;
				#if defined(MDEBUG) && MDEBUG
				bool early_exit = false;
				#endif /* MDEBUG */
				for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
					Problem* problem = problem_type->get_problem();

					RunHelper run_helper;
					#if defined(MDEBUG) && MDEBUG
					run_helper.starting_run_seed = run_index;
					run_helper.curr_run_seed = run_index;
					run_index++;
					#endif /* MDEBUG */

					vector<ContextLayer> context;
					ScopeHistory* scope_history = new ScopeHistory(duplicate->scopes[0]);
					duplicate->scopes[0]->activate(
						problem,
						context,
						run_helper,
						scope_history);
					delete scope_history;

					if (run_helper.num_actions > max_num_actions) {
						max_num_actions = run_helper.num_actions;
					}

					double target_val;
					if (!run_helper.exceeded_limit) {
						target_val = problem->score_result(run_helper.num_decisions,
														   run_helper.num_actions);
					} else {
						target_val = -1.0;
					}

					target_vals.push_back(target_val);

					delete problem;

					#if defined(MDEBUG) && MDEBUG
					if (run_index%2000 == 0) {
						early_exit = true;
						break;
					}
					#endif /* MDEBUG */
				}

				#if defined(MDEBUG) && MDEBUG
				if (early_exit) {
					delete duplicate;

					delete solution;
					solution = new Solution();
					solution->load("", "main");

					update_eval();

					continue;
				}
				#endif /* MDEBUG */

				double sum_score = 0.0;
				for (int d_index = 0; d_index < MEASURE_ITERS; d_index++) {
					sum_score += target_vals[d_index];
				}
				duplicate->average_score = sum_score / MEASURE_ITERS;

				duplicate->max_num_actions = max_num_actions;
				duplicate->num_actions_limit = 2*duplicate->max_num_actions + 10;

				cout << "duplicate->average_score: " << duplicate->average_score << endl;

				ofstream display_file;
				display_file.open("../display.txt");
				duplicate->save_for_display(display_file);
				display_file.close();

				duplicate->timestamp++;

				#if defined(MDEBUG) && MDEBUG
				delete solution;
				solution = duplicate;

				// solution->save("", "main");

				update_eval();
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
		}
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
