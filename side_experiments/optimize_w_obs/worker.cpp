#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
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
	if (argc != 2) {
		cout << "Usage: ./worker [path]" << endl;
		exit(1);
	}
	string path = argv[1];

	/**
	 * - worker directories need to have already been created
	 */

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->load("workers/", "main");
	solution->check_reset();

	auto start_time = chrono::high_resolution_clock::now();
	while (true) {
		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;

			cout << "alive" << endl;

			ifstream solution_save_file;
			solution_save_file.open("workers/saves/main.txt");
			string timestamp_line;
			getline(solution_save_file, timestamp_line);
			int curr_timestamp = stoi(timestamp_line);
			solution_save_file.close();

			if (curr_timestamp > solution->timestamp) {
				delete solution;

				solution = new Solution();
				solution->load("workers/", "main");
				solution->check_reset();
				cout << "updated from main" << endl;

				continue;
			}
		}

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		get_existing_result(problem,
							run_helper);

		if (run_helper.experiment_histories.size() > 0) {
			run_helper.num_analyze = 0;
			run_helper.num_actions = 0;

			vector<ContextLayer> context;
			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->experiment_activate(
				problem,
				context,
				run_helper,
				scope_history);

			delete scope_history;

			double target_val = problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;

			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
						+ run_helper.experiment_histories[0]->experiment->average_remaining_experiments_from_start);
			}

			run_helper.experiment_histories.back()->experiment->backprop(
				target_val,
				run_helper);
			if (run_helper.experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_FAIL) {
				run_helper.experiment_histories.back()->experiment->finalize(NULL);
				delete run_helper.experiment_histories.back()->experiment;
			} else if (run_helper.experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				/**
				 * - history->experiment_histories.size() == 1
				 */
				Solution* duplicate = new Solution(solution);

				int last_updated_scope_id = run_helper.experiment_histories.back()->experiment->scope_context->id;

				run_helper.experiment_histories.back()->experiment->finalize(duplicate);
				delete run_helper.experiment_histories.back()->experiment;

				Scope* experiment_scope = duplicate->scopes[last_updated_scope_id];
				clean_scope(experiment_scope,
							duplicate);

				double sum_score = 0.0;
				double sum_true_score = 0.0;
				bool early_exit = false;
				for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
					Problem* problem = problem_type->get_problem();

					RunHelper run_helper;

					vector<ContextLayer> context;
					duplicate->scopes[0]->activate(
						problem,
						context,
						run_helper);

					double target_val = problem->score_result();
					sum_score += target_val - 0.05 * run_helper.num_actions * solution->curr_time_penalty
						- run_helper.num_analyze * solution->curr_time_penalty;
					sum_true_score += target_val;

					delete problem;

					auto curr_time = chrono::high_resolution_clock::now();
					auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
					if (time_diff.count() >= 20) {
						start_time = curr_time;

						cout << "alive" << endl;

						ifstream solution_save_file;
						solution_save_file.open("workers/saves/main.txt");
						string timestamp_line;
						getline(solution_save_file, timestamp_line);
						int curr_timestamp = stoi(timestamp_line);
						solution_save_file.close();

						if (curr_timestamp > solution->timestamp) {
							early_exit = true;
							break;
						}
					}
				}

				if (early_exit) {
					delete duplicate;

					delete solution;

					solution = new Solution();
					solution->load("workers/", "main");
					solution->check_reset();
					cout << "updated from main" << endl;

					delete problem;

					continue;
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

				duplicate->timestamp++;

				if (duplicate->timestamp % INCREASE_TIME_PENALTY_ITER == 0) {
					duplicate->curr_time_penalty *= 1.25;
				}
				if ((duplicate->best_true_score_timestamp - duplicate->timestamp)
						% DECREASE_TIME_PENALTY_ITER == 0) {
					duplicate->curr_time_penalty *= 0.8;
				}
				if (duplicate->curr_true_score > duplicate->best_true_score) {
					duplicate->best_true_score = duplicate->curr_true_score;
					duplicate->best_true_score_timestamp = duplicate->timestamp;
				}

				duplicate->save(path, "possible_" + to_string((unsigned)time(NULL)));

				delete duplicate;
			}
		}

		delete problem;
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
