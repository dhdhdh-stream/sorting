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
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		cout << "Usage: ./worker [path] [filename]" << endl;
		exit(1);
	}
	string path = argv[1];
	string filename = argv[2];

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->load(path, filename);

	auto start_time = chrono::high_resolution_clock::now();

	while (solution->timestamp < EXPLORE_ITERS) {
		if (solution->timestamp % NEW_SCOPE_ITERS == 0) {
			if (solution->scopes[0]->child_scopes.size() == 0) {
				solution->scopes[0]->commit();
			} else {
				solution->scopes.back()->commit();
			}
		}

		Solution* best_solution = NULL;

		int improvement_iter = 0;

		while (true) {
			auto curr_time = chrono::high_resolution_clock::now();
			auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
			if (time_diff.count() >= 20) {
				start_time = curr_time;

				cout << "alive" << endl;
			}

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->experiment_activate(
					problem,
					run_helper,
					scope_history);

			double target_val = problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;

			if (run_helper.experiments_seen_order.size() == 0) {
				if ((solution->timestamp + 1) % NEW_SCOPE_ITERS == 0) {
					create_new_scope_experiment(scope_history);
				} else {
					create_branch_experiment(scope_history);
				}
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

					double sum_score = 0.0;
					double sum_true_score = 0.0;
					for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
						auto curr_time = chrono::high_resolution_clock::now();
						auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
						if (time_diff.count() >= 20) {
							start_time = curr_time;

							cout << "alive" << endl;
						}

						Problem* problem = problem_type->get_problem();

						RunHelper run_helper;

						ScopeHistory* scope_history = new ScopeHistory(duplicate->scopes[0]);
						duplicate->scopes[0]->activate(
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

					duplicate->curr_score = sum_score / MEASURE_ITERS;
					duplicate->curr_true_score = sum_true_score / MEASURE_ITERS;

					cout << "duplicate->curr_score: " << duplicate->curr_score << endl;

					duplicate->timestamp++;

					if (duplicate->timestamp % INCREASE_TIME_PENALTY_ITER == 0) {
						duplicate->curr_time_penalty *= 1.25;
					}
					if (duplicate->curr_true_score > duplicate->best_true_score) {
						duplicate->best_true_score = duplicate->curr_true_score;
						duplicate->best_true_score_timestamp = duplicate->timestamp;
					}
					if (duplicate->best_true_score_timestamp < duplicate->timestamp
							&& (duplicate->timestamp - duplicate->best_true_score_timestamp)
								% DECREASE_TIME_PENALTY_ITER == 0) {
						duplicate->curr_time_penalty *= 0.8;
					}

					if (best_solution == NULL
							|| duplicate->curr_score > best_solution->curr_score) {
						if (best_solution != NULL) {
							delete best_solution;
						}

						best_solution = duplicate;
					} else {
						delete duplicate;
					}

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

		delete solution;
		solution = best_solution;

		solution->save(path, filename);
	}

	solution->clean_scopes();

	solution->save(path, filename);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
