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
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "sorting.h"

using namespace std;

int seed;

default_random_engine generator;

Problem* problem_type;
Solution* solution;

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

	// problem_type = new Sorting();
	problem_type = new Minesweeper();

	solution = new Solution();
	solution->load("workers/", "main");

	auto start_time = chrono::high_resolution_clock::now();
	while (true) {
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		vector<ContextLayer> context;
		solution->scopes[0]->activate(
			problem,
			context,
			run_helper);

		if (run_helper.experiments_seen_order.size() == 0) {
			if (!run_helper.exceeded_limit) {
				create_experiment(run_helper.explore_node,
								  run_helper.explore_is_branch);
			}
		}

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_decisions);
		} else {
			target_val = -1.0;
		}

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
				delete solution;

				solution = new Solution();
				solution->load("workers/", "main");

				cout << "updated from main" << endl;

				continue;
			}
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
				 * - history->experiment_histories.size() == 1
				 */
				int experiment_scope_id = run_helper.experiment_histories.back()->experiment->scope_context->id;
				int new_scope_id = -1;
				if (run_helper.experiment_histories.back()->experiment->type == EXPERIMENT_TYPE_NEW_ACTION) {
					new_scope_id = (int)solution->scopes.size();
				}

				Solution* duplicate = new Solution(solution);
				run_helper.experiment_histories.back()->experiment->finalize(duplicate);
				delete run_helper.experiment_histories.back()->experiment;

				Scope* experiment_scope = duplicate->scopes[experiment_scope_id];
				Scope* new_scope = NULL;
				if (new_scope_id != -1) {
					new_scope = duplicate->scopes[new_scope_id];
				}

				clean_scope(experiment_scope);

				vector<double> o_target_val_histories;
				vector<ScopeHistory*> scope_histories;
				vector<double> target_val_histories;
				vector<ScopeHistory*> new_scope_histories;
				vector<double> new_target_val_histories;
				int max_num_actions = 0;
				bool early_exit = false;
				for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
					// Problem* problem = new Sorting();
					Problem* problem = new Minesweeper();

					RunHelper run_helper;

					Metrics metrics;
					metrics.experiment_scope = experiment_scope;
					metrics.new_scope = new_scope;

					vector<ContextLayer> context;
					duplicate->scopes[0]->measure_activate(
						metrics,
						problem,
						context,
						run_helper);

					if (run_helper.num_actions > max_num_actions) {
						max_num_actions = run_helper.num_actions;
					}

					double target_val;
					if (!run_helper.exceeded_limit) {
						target_val = problem->score_result(run_helper.num_decisions);
					} else {
						target_val = -1.0;
					}

					o_target_val_histories.push_back(target_val);

					for (int h_index = 0; h_index < (int)metrics.scope_histories.size(); h_index++) {
						scope_histories.push_back(metrics.scope_histories[h_index]);
						target_val_histories.push_back(target_val);
					}
					for (int h_index = 0; h_index < (int)metrics.new_scope_histories.size(); h_index++) {
						new_scope_histories.push_back(metrics.new_scope_histories[h_index]);
						new_target_val_histories.push_back(target_val);
					}

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
					for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
						delete scope_histories[h_index];
					}
					for (int h_index = 0; h_index < (int)new_scope_histories.size(); h_index++) {
						delete new_scope_histories[h_index];
					}

					delete duplicate;

					delete solution;

					solution = new Solution();
					solution->load("workers/", "main");

					cout << "updated from main" << endl;

					continue;
				}

				double sum_score = 0.0;
				for (int d_index = 0; d_index < MEASURE_ITERS; d_index++) {
					sum_score += o_target_val_histories[d_index];
				}
				duplicate->average_score = sum_score / MEASURE_ITERS;

				double sum_variance = 0.0;
				for (int d_index = 0; d_index < MEASURE_ITERS; d_index++) {
					sum_variance += (o_target_val_histories[d_index] - duplicate->average_score) * (o_target_val_histories[d_index] - duplicate->average_score);
				}
				duplicate->score_standard_deviation = sqrt(sum_variance / MEASURE_ITERS);
				if (duplicate->score_standard_deviation < MIN_STANDARD_DEVIATION) {
					duplicate->score_standard_deviation = MIN_STANDARD_DEVIATION;
				}

				duplicate->max_num_actions = max_num_actions;

				cout << "duplicate->average_score: " << duplicate->average_score << endl;

				update_eval(experiment_scope,
							scope_histories,
							target_val_histories);
				if (new_scope != NULL) {
					if (new_scope_histories.size() > 0) {
						update_eval(new_scope,
									new_scope_histories,
									new_target_val_histories);

						duplicate->timestamp++;
						duplicate->save(path, "possible_" + to_string((unsigned)time(NULL)));
					}
				} else {
					duplicate->timestamp++;
					duplicate->save(path, "possible_" + to_string((unsigned)time(NULL)));
				}

				delete duplicate;
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
