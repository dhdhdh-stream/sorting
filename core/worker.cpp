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

#include "action_node.h"
#include "branch_experiment.h"
#include "globals.h"
#include "minesweeper.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "sorting.h"

using namespace std;

const int NUM_FAILS_BEFORE_INCREASE = 100;

default_random_engine generator;

Problem* problem_type;
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

	// problem_type = new Sorting();
	problem_type = new Minesweeper();

	solution = new Solution();
	solution->load("workers/", "main");

	int num_fails = 0;

	auto start_time = chrono::high_resolution_clock::now();
	while (true) {
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = solution->root;
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(solution->root);
		context.back().scope_history = root_history;

		// unused
		int exit_depth = -1;
		AbstractNode* exit_node = NULL;

		solution->root->activate(solution->root->default_starting_node,
								 problem,
								 context,
								 exit_depth,
								 exit_node,
								 run_helper,
								 root_history);

		if (run_helper.experiments_seen_order.size() == 0) {
			if (!run_helper.exceeded_limit) {
				create_experiment(root_history);
			}
		}

		delete root_history;

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
		} else {
			target_val = -1.0;
		}

		bool is_success = false;
		bool is_fail = false;
		if (run_helper.experiment_histories.size() > 0) {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
						+ run_helper.experiment_histories[0]->experiment->average_remaining_experiments_from_start);
			}
			for (int h_index = 0; h_index < (int)run_helper.experiment_histories.size()-1; h_index++) {
				PassThroughExperimentHistory* pass_through_experiment_history = (PassThroughExperimentHistory*)run_helper.experiment_histories[h_index];
				for (int e_index = 0; e_index < (int)pass_through_experiment_history->experiments_seen_order.size(); e_index++) {
					AbstractExperiment* experiment = pass_through_experiment_history->experiments_seen_order[e_index];
					experiment->average_remaining_experiments_from_start =
						0.9 * experiment->average_remaining_experiments_from_start
						+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
							+ run_helper.experiment_histories[h_index+1]->experiment->average_remaining_experiments_from_start);
				}
			}

			run_helper.experiment_histories.back()->experiment->backprop(
				target_val,
				run_helper);
			if (run_helper.experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_FAIL) {
				if (run_helper.experiment_histories.size() == 1) {
					is_fail = true;
					run_helper.experiment_histories.back()->experiment->finalize();
					delete run_helper.experiment_histories.back()->experiment;
				} else {
					PassThroughExperiment* curr_experiment = run_helper.experiment_histories.back()->experiment->parent_experiment;

					curr_experiment->state_iter++;
					int matching_index;
					for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
						if (curr_experiment->child_experiments[c_index] == run_helper.experiment_histories.back()->experiment) {
							matching_index = c_index;
							break;
						}
					}
					curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

					run_helper.experiment_histories.back()->experiment->result = EXPERIMENT_RESULT_FAIL;
					run_helper.experiment_histories.back()->experiment->finalize();
					delete run_helper.experiment_histories.back()->experiment;

					double target_count = (double)MAX_PASS_THROUGH_EXPERIMENT_NUM_EXPERIMENTS
						* pow(0.5, run_helper.experiment_histories.size()-1);
					while (true) {
						if (curr_experiment == NULL) {
							is_fail = true;
							break;
						}

						if (curr_experiment->state_iter >= target_count) {
							PassThroughExperiment* parent = curr_experiment->parent_experiment;

							if (parent != NULL) {
								parent->state_iter++;
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
							curr_experiment->finalize();
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
				 * - run_helper.experiment_histories.size() == 1
				 */
				is_success = true;
				run_helper.experiment_histories.back()->experiment->finalize();
				delete run_helper.experiment_histories.back()->experiment;
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
			solution_save_file.open("workers/saves/main.txt");
			string timestamp_line;
			getline(solution_save_file, timestamp_line);
			int curr_timestamp = stoi(timestamp_line);
			solution_save_file.close();

			if (curr_timestamp > solution->timestamp) {
				delete solution;

				solution = new Solution();
				solution->load("workers/", "main");

				solution->save(path, name);

				cout << "updated from main" << endl;
			} else {
				if (solution->max_depth < 50) {
					solution->depth_limit = solution->max_depth + 10;
				} else {
					solution->depth_limit = (int)(1.2*(double)solution->max_depth);
				}

				solution->num_actions_limit = 20*solution->max_num_actions + 20;

				solution->timestamp = (unsigned)time(NULL);
				solution->save(path, name);
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

					num_fails = 0;
				}

				start_time = curr_time;
			}
		}
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
