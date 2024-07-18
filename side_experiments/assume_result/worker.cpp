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
#include "solution_set.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
SolutionSet* solution_set;

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

	solution_set = new SolutionSet();
	solution_set->load("workers/", "main");

	solution_set->increment();

	auto start_time = chrono::high_resolution_clock::now();
	while (true) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		run_helper.result = get_existing_result(problem);

		vector<ContextLayer> context;
		Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
		solution->scopes[0]->activate(
			problem,
			context,
			run_helper);

		if (run_helper.experiments_seen_order.size() == 0) {
			if (!run_helper.exceeded_limit) {
				create_experiment(run_helper);
			}
		}

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_analyze,
											   run_helper.num_actions);
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

			if (curr_timestamp > solution_set->timestamp) {
				delete solution_set;

				solution_set = new SolutionSet();
				solution_set->load("workers/", "main");
				cout << "updated from main" << endl;

				solution_set->increment();

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
				SolutionSet* duplicate = new SolutionSet(solution_set);
				Solution* duplicate_solution = duplicate->solutions[duplicate->curr_solution_index];

				duplicate_solution->last_updated_scope_id = run_helper.experiment_histories.back()->experiment->scope_context->id;
				if (run_helper.experiment_histories.back()->experiment->type == EXPERIMENT_TYPE_NEW_ACTION) {
					duplicate_solution->last_new_scope_id = (int)duplicate_solution->scopes.size();
				}

				run_helper.experiment_histories.back()->experiment->finalize(duplicate_solution);
				delete run_helper.experiment_histories.back()->experiment;

				Scope* experiment_scope = duplicate_solution->scopes[duplicate_solution->last_updated_scope_id];
				clean_scope(experiment_scope);

				vector<double> target_vals;
				int max_num_actions = 0;
				bool early_exit = false;
				for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
					Problem* problem = problem_type->get_problem();

					RunHelper run_helper;

					vector<ContextLayer> context;
					duplicate_solution->scopes[0]->measure_activate(
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

						if (curr_timestamp > solution_set->timestamp) {
							early_exit = true;
							break;
						}
					}
				}

				if (early_exit) {
					delete duplicate;

					delete solution_set;

					solution_set = new SolutionSet();
					solution_set->load("workers/", "main");
					cout << "updated from main" << endl;

					solution_set->increment();

					continue;
				}

				for (int s_index = 0; s_index < (int)duplicate_solution->scopes.size(); s_index++) {
					for (map<int, AbstractNode*>::iterator it = duplicate_solution->scopes[s_index]->nodes.begin();
							it != duplicate_solution->scopes[s_index]->nodes.end(); it++) {
						it->second->average_instances_per_run /= MEASURE_ITERS;
						if (it->second->average_instances_per_run < 1.0) {
							it->second->average_instances_per_run = 1.0;
						}
					}
				}

				double sum_score = 0.0;
				for (int d_index = 0; d_index < MEASURE_ITERS; d_index++) {
					sum_score += target_vals[d_index];
				}
				duplicate->average_score = sum_score / MEASURE_ITERS;

				duplicate_solution->max_num_actions = max_num_actions;

				cout << "duplicate->average_score: " << duplicate->average_score << endl;

				duplicate->timestamp++;

				duplicate->save(path, "possible_" + to_string((unsigned)time(NULL)));

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
	delete solution_set;

	cout << "Done" << endl;
}
