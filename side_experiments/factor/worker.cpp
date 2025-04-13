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

int multi_index = 0;

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

	run_index = 0;
	auto start_time = chrono::high_resolution_clock::now();

	while (solution->timestamp < EXPLORE_ITERS) {
		AbstractExperiment* curr_experiment = NULL;
		AbstractExperiment* best_experiment = NULL;

		int improvement_iter = 0;

		while (true) {
			run_index++;
			auto curr_time = chrono::high_resolution_clock::now();
			auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
			if (time_diff.count() >= 20) {
				start_time = curr_time;

				cout << "improvement_iter: " << improvement_iter << endl;
			}

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->experiment_activate(
					problem,
					run_helper,
					scope_history);

			if (run_helper.keypoint_misguess_factors.size() > 0) {
				double sum_misguess_factors = 0.0;
				for (int m_index = 0; m_index < (int)run_helper.keypoint_misguess_factors.size(); m_index++) {
					sum_misguess_factors += run_helper.keypoint_misguess_factors[m_index];
				}
				if (sum_misguess_factors / run_helper.keypoint_misguess_factors.size() > KEYPOINT_MAX_FACTOR) {
					run_helper.early_exit = true;
				}
			}

			double target_val;
			if (run_helper.early_exit) {
				target_val = -10.0;
			} else {
				double target_val = problem->score_result();
				target_val -= run_helper.num_actions * solution->curr_time_penalty;
			}

			if (curr_experiment == NULL) {
				create_experiment(scope_history,
								  improvement_iter,
								  curr_experiment);
			}

			delete scope_history;
			delete problem;

			if (run_helper.experiment_history != NULL) {
				run_helper.experiment_history->experiment->backprop(
					target_val,
					run_helper);
				if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
					run_helper.experiment_history->experiment->clean();
					delete run_helper.experiment_history->experiment;

					curr_experiment = NULL;
				} else if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
					run_helper.experiment_history->experiment->clean();

					if (best_experiment == NULL) {
						best_experiment = run_helper.experiment_history->experiment;
					} else {
						if (run_helper.experiment_history->experiment->improvement > best_experiment->improvement) {
							delete best_experiment;
							best_experiment = run_helper.experiment_history->experiment;
						} else {
							delete run_helper.experiment_history->experiment;
						}
					}

					curr_experiment = NULL;

					improvement_iter++;
					if (improvement_iter >= IMPROVEMENTS_PER_ITER) {
						break;
					}
				}
			}
		}

		Scope* last_updated_scope = best_experiment->scope_context;

		best_experiment->add();
		delete best_experiment;

		clean_scope(last_updated_scope);

		if (last_updated_scope->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
			last_updated_scope->exceeded = true;
		}
		if (last_updated_scope->nodes.size() <= SCOPE_RESUME_NUM_NODES) {
			last_updated_scope->exceeded = false;
		}

		solution->clean();

		double sum_score = 0.0;
		double sum_true_score = 0.0;
		vector<ScopeHistory*> scope_histories;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			run_index++;

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->measure_activate(
				problem,
				run_helper,
				scope_history);
			delete scope_history;

			double target_val = problem->score_result();
			sum_score += target_val - run_helper.num_actions * solution->curr_time_penalty;
			sum_true_score += target_val;

			update_scores(scope_history,
						  target_val);

			scope_histories.push_back(scope_history);

			delete problem;
		}

		solution->measure_update();

		for (int k_index = 0; k_index < KEYPOINT_EXPERIMENTS_PER_MEASURE; k_index++) {
			keypoint_experiment(scope_histories);
		}

		for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
			delete scope_histories[h_index];
		}

		solution->curr_score = sum_score / MEASURE_ITERS;
		solution->curr_true_score = sum_true_score / MEASURE_ITERS;

		cout << "solution->curr_score: " << solution->curr_score << endl;

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

		solution->save(path, filename);
	}

	solution->clean_scopes();
	solution->save(path, filename);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
