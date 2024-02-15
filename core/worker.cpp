/**
 * - TODO: to parallelize further:
 *   - have new instances that don't sync, but draw from existing
 *     - initially, they may be much worse, but if they ever become better, swap to
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "increment_minesweeper.h"
#include "outer_experiment.h"
#include "scope.h"
#include "simple.h"
#include "solution.h"
#include "solution_helpers.h"
#include "sorting.h"

using namespace std;

const int NUM_FAILS_BEFORE_INCREASE = 30;

default_random_engine generator;

Solution* solution;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		cout << "Usage: ./worker [path] [name]" << endl;
		exit(1);
	}
	string path = argv[1];
	string name = argv[2];

	/**
	 * - worker directories need to have already been created
	 */

	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->load(path, "main");

	int num_fails = 0;

	auto start_time = chrono::high_resolution_clock::now();
	while (true) {
		// Problem* problem = new Sorting();
		Problem* problem = new IncrementMinesweeper();
		// Problem* problem = new Simple();

		RunHelper run_helper;

		bool outer_is_selected = false;
		if (solution->outer_experiment != NULL) {
			outer_is_selected = solution->outer_experiment->activate(
				problem,
				run_helper);
		}
		if (!outer_is_selected) {
			vector<ContextLayer> context;
			context.push_back(ContextLayer());

			context.back().scope = solution->root;
			context.back().node = NULL;

			ScopeHistory* root_history = new ScopeHistory(solution->root);
			context.back().scope_history = root_history;

			// unused
			int exit_depth = -1;
			AbstractNode* exit_node = NULL;

			solution->root->activate(problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper,
									 root_history);

			if (run_helper.experiment_history == NULL) {
				if (run_helper.experiments_seen_order.size() == 0) {
					if (!run_helper.exceeded_limit) {
						create_experiment(root_history);
					}
				}
			}

			delete root_history;
		}

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
		} else {
			target_val = -1.0;
		}

		bool is_success = false;
		bool is_fail = false;
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
				run_helper,
				run_helper.experiment_history);
			if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
				is_fail = true;
				run_helper.experiment_history->experiment->finalize();
				delete run_helper.experiment_history->experiment;
			} else if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				is_success = true;
				run_helper.experiment_history->experiment->finalize();
				delete run_helper.experiment_history->experiment;
			}
		} else {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index);
			}
		}

		delete problem;

		if (is_success) {
			solution->success_reset();

			ifstream solution_save_file;
			solution_save_file.open(path + "saves/main.txt");
			string timestamp_line;
			getline(solution_save_file, timestamp_line);
			int curr_timestamp = stoi(timestamp_line);
			solution_save_file.close();

			if (curr_timestamp > solution->timestamp) {
				delete solution;

				solution = new Solution();
				solution->load(path, "main");

				cout << "updated from main" << endl;
			} else {
				/**
				 * - possible race condition
				 *   - but just means that previous update from another worker dropped
				 */
				solution->timestamp = (unsigned)time(NULL);
				solution->save(path, name);

				if (solution->max_depth < 50) {
					solution->depth_limit = solution->max_depth + 10;
				} else {
					solution->depth_limit = (int)(1.2*(double)solution->max_depth);
				}
			}

			num_fails = 0;

			solution->curr_num_datapoints = STARTING_NUM_DATAPOINTS;
		} else if (is_fail) {
			num_fails++;
			cout << "num_fails: " << num_fails << endl << endl;
			if (num_fails > NUM_FAILS_BEFORE_INCREASE) {
				cout << "fail_reset" << endl << endl;

				num_fails = 0;
				solution->fail_reset();

				solution->curr_num_datapoints *= 2;
			}
		} else {
			auto curr_time = chrono::high_resolution_clock::now();
			auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
			if (time_diff.count() >= 10) {
				cout << "alive" << endl;

				ifstream solution_save_file;
				solution_save_file.open(path + "saves/main.txt");
				string timestamp_line;
				getline(solution_save_file, timestamp_line);
				int curr_timestamp = stoi(timestamp_line);
				solution_save_file.close();

				if (curr_timestamp > solution->timestamp) {
					delete solution;

					solution = new Solution();
					solution->load(path, "main");

					cout << "updated from main" << endl;

					num_fails = 0;
				}

				start_time = curr_time;
			}
		}
	}

	delete solution;

	cout << "Done" << endl;
}
