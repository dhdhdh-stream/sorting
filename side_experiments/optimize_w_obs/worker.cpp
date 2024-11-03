#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_network.h"
#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "lstm.h"
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

vector<LSTM*> mimic_memory_cells;
vector<ActionNetwork*> mimic_action_networks;

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

	mimic_memory_cells = vector<LSTM*>(MIMIC_NUM_STATES);
	for (int s_index = 0; s_index < MIMIC_NUM_STATES; s_index++) {
		ifstream memory_cell_save_file;
		memory_cell_save_file.open("workers/saves/mimic/memory_cell_" + to_string(s_index) + ".txt");
		mimic_memory_cells[s_index] = new LSTM(memory_cell_save_file);
		memory_cell_save_file.close();
		mimic_memory_cells[s_index]->index = s_index;
	}
	mimic_action_networks = vector<ActionNetwork*>(problem_type->num_possible_actions() + 1);
	for (int a_index = 0; a_index < problem_type->num_possible_actions() + 1; a_index++) {
		ifstream action_network_save_file;
		action_network_save_file.open("workers/saves/mimic/action_network_" + to_string(a_index) + ".txt");
		mimic_action_networks[a_index] = new ActionNetwork(action_network_save_file);
		action_network_save_file.close();
	}

	auto start_time = chrono::high_resolution_clock::now();
	while (true) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		run_helper.result = get_existing_result(problem);

		vector<ContextLayer> context;
		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->experiment_activate(
			problem,
			context,
			run_helper,
			scope_history);

		if (run_helper.experiments_seen_order.size() == 0) {
			if (!run_helper.exceeded_limit) {
				create_experiment(scope_history);
			}
		}

		delete scope_history;

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

				vector<double> target_vals;
				int max_num_actions = 0;
				bool early_exit = false;
				for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
					Problem* problem = problem_type->get_problem();

					RunHelper run_helper;

					vector<ContextLayer> context;
					duplicate->scopes[0]->activate(
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
					cout << "updated from main" << endl;

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

				double sum_score = 0.0;
				for (int d_index = 0; d_index < MEASURE_ITERS; d_index++) {
					sum_score += target_vals[d_index];
				}
				duplicate->average_score = sum_score / MEASURE_ITERS;

				duplicate->max_num_actions = max_num_actions;

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
	delete solution;

	cout << "Done" << endl;
}
