#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "eval.h"
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
		context.push_back(ContextLayer());

		context.back().scope = solution->scopes[0];
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		context.back().scope_history = root_history;

		solution->scopes[0]->activate(
			problem,
			context,
			run_helper,
			root_history);

		delete root_history;

		delete problem;

		if (run_helper.success_duplicate != NULL) {
			Solution* duplicate = run_helper.success_duplicate;
			run_helper.success_duplicate = NULL;

			while (true) {
				double sum_timestamp_score = 0.0;
				int timestamp_score_count = 0;
				double sum_vals = 0.0;
				double sum_instances_per_run = 0;
				double sum_local_num_actions = 0.0;
				int num_runs = 0;
				while (true) {
					// Problem* problem = new Sorting();
					Problem* problem = new Minesweeper();

					RunHelper run_helper;
					Metrics metrics(solution->explore_id,
									solution->explore_type,
									duplicate->explore_id,
									duplicate->explore_type);

					vector<ContextLayer> context;
					context.push_back(ContextLayer());

					context.back().scope = duplicate->scopes[0];
					context.back().node = NULL;

					ScopeHistory* root_history = new ScopeHistory(duplicate->scopes[0]);
					context.back().scope_history = root_history;

					duplicate->scopes[0]->measure_activate(
						metrics,
						problem,
						context,
						run_helper,
						root_history);

					delete root_history;

					double target_val;
					if (run_helper.num_actions > solution->num_actions_limit) {
						target_val = -1.0;
					} else {
						target_val = problem->score_result(run_helper.num_decisions);
					}

					sum_timestamp_score += metrics.curr_sum_timestamp_score;
					timestamp_score_count += metrics.curr_num_instances;
					sum_vals += target_val;
					if (run_helper.num_actions > duplicate->max_num_actions) {
						duplicate->max_num_actions = run_helper.num_actions;
					}
					sum_instances_per_run += metrics.next_num_instances;
					if (metrics.next_max_num_actions > duplicate->explore_scope_max_num_actions) {
						duplicate->explore_scope_max_num_actions = metrics.next_max_num_actions;
					}
					sum_local_num_actions += metrics.next_local_sum_num_actions;
					num_runs++;

					delete problem;

					if (timestamp_score_count > MEASURE_ITERS) {
						break;
					}
				}

				if (sum_instances_per_run > 0) {
					duplicate->timestamp_score = sum_timestamp_score / timestamp_score_count;
					duplicate->curr_average_score = sum_vals / num_runs;
					duplicate->explore_average_instances_per_run = (double)sum_instances_per_run / (double)num_runs;
					duplicate->explore_scope_local_average_num_actions = sum_local_num_actions / sum_instances_per_run;

					break;
				} else {
					uniform_int_distribution<int> explore_id_distribution(0, (int)duplicate->scopes.size()-1);
					duplicate->explore_id = explore_id_distribution(generator);
					if (duplicate->scopes[duplicate->explore_id]->eval->score_input_node_contexts.size() == 0) {
						duplicate->explore_type = EXPLORE_TYPE_EVAL;
					} else {
						uniform_int_distribution<int> explore_type_distribution(0, 1);
						duplicate->explore_type = explore_type_distribution(generator);
					}
					duplicate->explore_scope_max_num_actions = 1;
				}
			}

			duplicate->timestamp++;

			duplicate->save(path, "possible_" + to_string((unsigned)time(NULL)));

			delete duplicate;
		}

		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
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
			}

			start_time = curr_time;
		}
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
