#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
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

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	string filename;
	if (argc > 1) {
		filename = argv[1];
		solution->load("saves/", filename);
	} else {
		filename = "main.txt";
		solution->init();

		while (true) {
			run_index++;
			if (run_index%100000 == 0) {
				cout << "run_index: " << run_index << endl;
				cout << "solution->timestamp: " << solution->timestamp << endl;
			}

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			#if defined(MDEBUG) && MDEBUG
			run_helper.starting_run_seed = run_index;
			run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
			#endif /* MDEBUG */

			vector<ContextLayer> context;
			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->experiment_activate(
					problem,
					context,
					run_helper,
					scope_history);

			double target_val = problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;

			if (run_helper.experiments_seen_order.size() == 0) {
				create_commit_experiment(scope_history);
			}

			delete scope_history;
			delete problem;

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
					run_helper);
				if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
					run_helper.experiment_history->experiment->finalize(NULL);
					delete run_helper.experiment_history->experiment;
				} else if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
					int last_updated_scope_id = run_helper.experiment_history->experiment->scope_context->id;

					run_helper.experiment_history->experiment->finalize(solution);
					delete run_helper.experiment_history->experiment;

					Scope* experiment_scope = solution->scopes[last_updated_scope_id];
					clean_scope(experiment_scope,
								solution);
					solution->clean_scopes();

					solution->clear_experiments();

					ofstream display_file;
					display_file.open("../display.txt");
					solution->save_for_display(display_file);
					display_file.close();

					break;
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

		// solution->save("saves/", filename);
	}

	{
		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();
	}

	run_index = 0;

	while (solution->timestamp < EXPLORE_ITERS) {
		Solution* best_solution = NULL;

		int improvement_iter = 0;

		while (true) {
			run_index++;
			if (run_index%100000 == 0) {
				cout << "run_index: " << run_index << endl;
				cout << "solution->timestamp: " << solution->timestamp << endl;
				cout << "improvement_iter: " << improvement_iter << endl;
			}

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			#if defined(MDEBUG) && MDEBUG
			run_helper.starting_run_seed = run_index;
			run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
			#endif /* MDEBUG */

			vector<ContextLayer> context;
			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->experiment_activate(
					problem,
					context,
					run_helper,
					scope_history);

			double target_val = problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;

			if (!run_helper.has_explore) {
				update_scores(scope_history,
							  target_val);
			}

			if (run_helper.experiments_seen_order.size() == 0) {
				create_experiment(scope_history);
			}

			delete scope_history;
			delete problem;

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
					run_helper);
				if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
					run_helper.experiment_history->experiment->finalize(NULL);
					delete run_helper.experiment_history->experiment;
				} else if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
					Solution* duplicate = new Solution(solution);

					int last_updated_scope_id = run_helper.experiment_history->experiment->scope_context->id;

					run_helper.experiment_history->experiment->finalize(duplicate);
					delete run_helper.experiment_history->experiment;

					Scope* experiment_scope = duplicate->scopes[last_updated_scope_id];
					clean_scope(experiment_scope,
								duplicate);
					duplicate->clean_scopes();

					#if defined(MDEBUG) && MDEBUG
					while (duplicate->verify_problems.size() > 0) {
						Problem* problem = duplicate->verify_problems[0];

						RunHelper run_helper;
						run_helper.starting_run_seed = duplicate->verify_seeds[0];
						cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
						run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
						duplicate->verify_seeds.erase(duplicate->verify_seeds.begin());

						vector<ContextLayer> context;
						ScopeHistory* scope_history = new ScopeHistory(duplicate->scopes[0]);
						duplicate->scopes[0]->verify_activate(
							problem,
							context,
							run_helper,
							scope_history);
						delete scope_history;

						delete duplicate->verify_problems[0];
						duplicate->verify_problems.erase(duplicate->verify_problems.begin());
					}
					duplicate->clear_verify();
					#endif /* MDEBUG */

					double sum_score = 0.0;
					double sum_true_score = 0.0;
					for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
						Problem* problem = problem_type->get_problem();

						RunHelper run_helper;
						#if defined(MDEBUG) && MDEBUG
						run_helper.starting_run_seed = run_index;
						run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
						run_index++;
						#endif /* MDEBUG */

						vector<ContextLayer> context;
						ScopeHistory* scope_history = new ScopeHistory(duplicate->scopes[0]);
						duplicate->scopes[0]->measure_activate(
							problem,
							context,
							run_helper,
							scope_history);
						delete scope_history;

						double target_val = problem->score_result();
						sum_score += target_val - 0.05 * run_helper.num_actions * solution->curr_time_penalty
							- run_helper.num_analyze * solution->curr_time_penalty;
						sum_true_score += target_val;

						delete problem;
					}

					for (int s_index = 0; s_index < (int)duplicate->scopes.size(); s_index++) {
						for (map<int, AbstractNode*>::iterator it = duplicate->scopes[s_index]->nodes.begin();
								it != duplicate->scopes[s_index]->nodes.end(); it++) {
							it->second->average_instances_per_run /= MEASURE_ITERS;
							if (it->second->average_instances_per_run < 1.0) {
								it->second->average_instances_per_run = 1.0;
							}
						}
					}

					duplicate->curr_score = sum_score / MEASURE_ITERS;
					duplicate->curr_true_score = sum_true_score / MEASURE_ITERS;

					cout << "duplicate->curr_score: " << duplicate->curr_score << endl;

					ofstream display_file;
					display_file.open("../display.txt");
					duplicate->save_for_display(display_file);
					display_file.close();

					duplicate->timestamp++;

					if (duplicate->timestamp % INCREASE_TIME_PENALTY_ITER == 0) {
						duplicate->curr_time_penalty *= 1.25;
					}
					if (duplicate->curr_true_score > duplicate->best_true_score) {
						duplicate->best_true_score = duplicate->curr_true_score;
						duplicate->best_true_score_timestamp = duplicate->timestamp;
					}
					if ((duplicate->best_true_score_timestamp - duplicate->timestamp)
							% DECREASE_TIME_PENALTY_ITER == 0) {
						duplicate->curr_time_penalty *= 0.8;
					}

					#if defined(MDEBUG) && MDEBUG
					if (best_solution == NULL) {
					#else
					if (best_solution == NULL
							|| duplicate->curr_score > best_solution->curr_score) {
					#endif /* MDEBUG */
						if (best_solution != NULL) {
							delete best_solution;
						}

						best_solution = duplicate;
					} else {
						delete duplicate;
					}

					improvement_iter++;
					if (solution->was_commit) {
						if (improvement_iter >= COMMIT_IMPROVEMENTS_PER_ITER) {
							break;
						}
					} else {
						if (improvement_iter >= IMPROVEMENTS_PER_ITER) {
							break;
						}
					}
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

		delete solution;
		solution = best_solution;

		if (solution->timestamp % COMMIT_ITERS == 0
				&& solution->timestamp != EXPLORE_ITERS) {
			while (true) {
				run_index++;
				if (run_index%100000 == 0) {
					cout << "run_index: " << run_index << endl;
					cout << "solution->timestamp: " << solution->timestamp << endl;
				}

				Problem* problem = problem_type->get_problem();

				RunHelper run_helper;

				#if defined(MDEBUG) && MDEBUG
				run_helper.starting_run_seed = run_index;
				run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
				#endif /* MDEBUG */

				vector<ContextLayer> context;
				ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
				solution->scopes[0]->experiment_activate(
						problem,
						context,
						run_helper,
						scope_history);

				double target_val = problem->score_result();
				target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
				target_val -= run_helper.num_analyze * solution->curr_time_penalty;

				if (run_helper.experiments_seen_order.size() == 0) {
					create_commit_experiment(scope_history);
				}

				delete scope_history;
				delete problem;

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
						run_helper);
					if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
						run_helper.experiment_history->experiment->finalize(NULL);
						delete run_helper.experiment_history->experiment;
					} else if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
						int last_updated_scope_id = run_helper.experiment_history->experiment->scope_context->id;

						run_helper.experiment_history->experiment->finalize(solution);
						delete run_helper.experiment_history->experiment;

						Scope* experiment_scope = solution->scopes[last_updated_scope_id];
						clean_scope(experiment_scope,
									solution);
						solution->clean_scopes();

						solution->clear_experiments();

						ofstream display_file;
						display_file.open("../display.txt");
						solution->save_for_display(display_file);
						display_file.close();

						break;
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
		}

		// solution->save("saves/", filename);

		#if defined(MDEBUG) && MDEBUG
		delete solution;
		solution = new Solution();
		solution->load("saves/", filename);
		#endif /* MDEBUG */
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
