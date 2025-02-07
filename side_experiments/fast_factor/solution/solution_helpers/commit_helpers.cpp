#include "solution_helpers.h"

#include <chrono>
#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void commit_helper() {
	Solution* commit_start = new Solution(solution);

	auto start_time = chrono::high_resolution_clock::now();

	while (true) {
		run_index++;

		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;

			cout << "alive" << endl;
		}

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		ScopeHistory* scope_history = new ScopeHistory(commit_start->scopes[0]);
		commit_start->scopes[0]->experiment_activate(
				problem,
				run_helper,
				scope_history);

		double target_val = problem->score_result();
		target_val -= 0.05 * run_helper.num_actions * commit_start->curr_time_penalty;
		target_val -= run_helper.num_analyze * commit_start->curr_time_penalty;

		if (run_helper.experiments_seen_order.size() == 0) {
			create_commit_experiment(commit_start,
									 scope_history);
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

				run_helper.experiment_history->experiment->finalize(commit_start);
				delete run_helper.experiment_history->experiment;

				Scope* experiment_scope = commit_start->scopes[last_updated_scope_id];
				clean_scope(experiment_scope,
							commit_start);
				commit_start->clean_scopes();

				commit_start->clear_experiments();

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

	Solution* best_solution = NULL;

	int improvement_iter = 0;

	while (true) {
		run_index++;

		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;

			cout << "alive" << endl;
		}

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		ScopeHistory* scope_history = new ScopeHistory(commit_start->scopes[0]);
		commit_start->scopes[0]->experiment_activate(
				problem,
				run_helper,
				scope_history);

		double target_val = problem->score_result();
		target_val -= 0.05 * run_helper.num_actions * commit_start->curr_time_penalty;
		target_val -= run_helper.num_analyze * commit_start->curr_time_penalty;

		if (run_helper.experiments_seen_order.size() == 0) {
			create_experiment(commit_start,
							  scope_history);
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
				Solution* duplicate = new Solution(commit_start);

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

					ScopeHistory* scope_history = new ScopeHistory(duplicate->scopes[0]);
					duplicate->scopes[0]->verify_activate(
						problem,
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

					ScopeHistory* scope_history = new ScopeHistory(duplicate->scopes[0]);
					duplicate->scopes[0]->activate(
						problem,
						run_helper,
						scope_history);
					delete scope_history;

					double target_val = problem->score_result();
					sum_score += target_val - 0.05 * run_helper.num_actions * duplicate->curr_time_penalty
						- run_helper.num_analyze * duplicate->curr_time_penalty;
					sum_true_score += target_val;

					delete problem;
				}

				duplicate->curr_score = sum_score / MEASURE_ITERS;
				duplicate->curr_true_score = sum_true_score / MEASURE_ITERS;

				cout << "duplicate->curr_score: " << duplicate->curr_score << endl;

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
				if (improvement_iter >= COMMIT_IMPROVEMENTS_PER_ITER) {
					break;
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

	delete commit_start;
	if (best_solution->curr_score < solution->curr_score) {
		delete best_solution;
	} else {
		delete solution;
		solution = best_solution;
	}
}
