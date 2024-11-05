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
				cout << "updated from main" << endl;

				continue;
			}
		}

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		get_existing_result(problem,
							run_helper);

		if (run_helper.experiment_histories.size() > 0) {
			run_helper.exceeded_limit = false;

			run_helper.num_analyze = 0;
			run_helper.num_actions = 0;

			vector<ContextLayer> context;
			solution->scopes[0]->activate(
				problem,
				context,
				run_helper);

			double target_val;
			if (!run_helper.exceeded_limit) {
				target_val = problem->score_result(run_helper.num_analyze,
												   run_helper.num_actions);
			} else {
				target_val = -1.0;
			}

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
				clean_scope(experiment_scope);
				duplicate->clean();

				vector<double> target_vals;
				int max_num_actions = 0;
				bool measure_early_exit = false;
				for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
					Problem* problem = problem_type->get_problem();

					RunHelper run_helper;

					vector<ContextLayer> context;
					duplicate->scopes[0]->measure_activate(
						problem,
						context,
						run_helper);

					if (run_helper.num_actions > max_num_actions) {
						max_num_actions = run_helper.num_actions;
					}

					double target_val;
					if (!run_helper.exceeded_limit) {
						target_val = problem->score_result(run_helper.num_analyze,
														   run_helper.num_actions);
					} else {
						target_val = -1.0;
					}

					target_vals.push_back(target_val);

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
							measure_early_exit = true;
							break;
						}
					}
				}

				if (measure_early_exit) {
					delete duplicate;

					delete solution;

					solution = new Solution();
					solution->load("workers/", "main");
					cout << "updated from main" << endl;

					delete problem;

					continue;
				}

				double sum_score_difference = 0.0;
				int branch_count = 0;
				bool flip_early_exit = false;
				for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
					Problem* problem = problem_type->get_problem();

					vector<int> branch_node_indexes;
					double original_score;
					{
						Problem* copy_problem = problem->copy_and_reset();

						RunHelper run_helper;

						vector<ContextLayer> context;
						duplicate->scopes[0]->flip_gather_activate(
							copy_problem,
							context,
							run_helper,
							branch_node_indexes);

						if (!run_helper.exceeded_limit) {
							original_score = copy_problem->score_result(run_helper.num_analyze,
																		run_helper.num_actions);
						} else {
							original_score = -1.0;
						}

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
								flip_early_exit = true;
								break;
							}
						}
					}

					for (int b_index = 0; b_index < (int)branch_node_indexes.size(); b_index++) {
						Problem* copy_problem = problem->copy_and_reset();

						RunHelper run_helper;

						vector<ContextLayer> context;
						duplicate->scopes[0]->flip_activate(
							copy_problem,
							context,
							run_helper,
							branch_node_indexes[b_index]);

						double target_val;
						if (!run_helper.exceeded_limit) {
							target_val = copy_problem->score_result(run_helper.num_analyze,
																	run_helper.num_actions);
						} else {
							target_val = -1.0;
						}

						sum_score_difference += (original_score - target_val);
						branch_count++;

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
								flip_early_exit = true;
								break;
							}
						}
					}

					if (flip_early_exit) {
						break;
					}
				}

				if (flip_early_exit) {
					delete duplicate;

					delete solution;
					solution = new Solution();
					solution->load("", "main");

					delete problem;

					continue;
				}

				for (int s_index = 0; s_index < (int)duplicate->scopes.size(); s_index++) {
					for (map<int, AbstractNode*>::iterator it = duplicate->scopes[s_index]->nodes.begin();
							it != duplicate->scopes[s_index]->nodes.end(); it++) {
						it->second->average_instances_per_run /= MEASURE_ITERS;
					}
				}

				double sum_score = 0.0;
				for (int d_index = 0; d_index < MEASURE_ITERS; d_index++) {
					sum_score += target_vals[d_index];
				}
				duplicate->average_score = sum_score / MEASURE_ITERS;
				duplicate->decision_quality = sum_score_difference / branch_count;

				duplicate->max_num_actions = max_num_actions;

				cout << "duplicate->average_score: " << duplicate->average_score << endl;

				duplicate->timestamp++;

				duplicate->update_subproblem();

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
