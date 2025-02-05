#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
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

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->activate(
				problem,
				run_helper,
				scope_history);
			delete scope_history;

			double target_val = problem->score_result();
			sum_score += target_val - 0.05 * run_helper.num_actions * solution->curr_time_penalty
				- run_helper.num_analyze * solution->curr_time_penalty;
			sum_true_score += target_val;

			delete problem;
		}

		solution->curr_score = sum_score / MEASURE_ITERS;
		solution->curr_true_score = sum_true_score / MEASURE_ITERS;

		commit_helper();

		solution->save("saves/", filename);
	}

	{
		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();
	}

	run_index = 0;

	while (solution->timestamp < EXPLORE_ITERS) {
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

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->experiment_activate(
					problem,
					run_helper,
					scope_history);

			double target_val = problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;

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
					Scope* experiment_scope = run_helper.experiment_history->experiment->scope_context;

					run_helper.experiment_history->experiment->finalize(solution);
					delete run_helper.experiment_history->experiment;

					clean_scope(experiment_scope,
								solution);
					solution->clean_scopes();

					#if defined(MDEBUG) && MDEBUG
					while (solution->verify_problems.size() > 0) {
						Problem* problem = solution->verify_problems[0];

						RunHelper run_helper;
						run_helper.starting_run_seed = solution->verify_seeds[0];
						cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
						run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
						solution->verify_seeds.erase(solution->verify_seeds.begin());

						ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
						solution->scopes[0]->verify_activate(
							problem,
							run_helper,
							scope_history);
						delete scope_history;

						delete solution->verify_problems[0];
						solution->verify_problems.erase(solution->verify_problems.begin());
					}
					solution->clear_verify();
					#endif /* MDEBUG */

					solution->clear_experiments();

					improvement_iter++;
					if (improvement_iter >= IMPROVEMENTS_PER_ITER) {
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

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->activate(
				problem,
				run_helper,
				scope_history);
			delete scope_history;

			double target_val = problem->score_result();
			sum_score += target_val - 0.05 * run_helper.num_actions * solution->curr_time_penalty
				- run_helper.num_analyze * solution->curr_time_penalty;
			sum_true_score += target_val;

			delete problem;
		}

		solution->curr_score = sum_score / MEASURE_ITERS;
		solution->curr_true_score = sum_true_score / MEASURE_ITERS;

		cout << "solution->curr_score: " << solution->curr_score << endl;

		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();

		solution->timestamp++;

		if (solution->timestamp % INCREASE_TIME_PENALTY_ITER == 0) {
			solution->curr_time_penalty *= 1.25;
		}
		if (solution->curr_true_score > solution->best_true_score) {
			solution->best_true_score = solution->curr_true_score;
			solution->best_true_score_timestamp = solution->timestamp;
		}
		if (solution->best_true_score_timestamp < solution->timestamp
				&& (solution->timestamp - solution->best_true_score_timestamp)
					% DECREASE_TIME_PENALTY_ITER == 0) {
			solution->curr_time_penalty *= 0.8;
		}

		if (solution->timestamp % COMMIT_ITERS == 0
				&& solution->timestamp != EXPLORE_ITERS) {
			commit_helper();
		}

		solution->save("saves/", filename);

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
