#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
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

int num_actions_until_experiment = -1;
int num_actions_after_experiment_to_skip = -1;
bool eval_experiment;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "Usage: ./worker [path]" << endl;
		exit(1);
	}
	string path = argv[1];

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new Minesweeper();

	solution = new Solution();
	solution->load("workers/", "main");

	auto start_time = chrono::high_resolution_clock::now();
	while (true) {
		Minesweeper* problem = new Minesweeper();

		RunHelper run_helper;

		geometric_distribution<int> iter_distribution(0.5);
		int num_iters = 1 + iter_distribution(generator);

		for (int i_index = 0; i_index < num_iters; i_index++) {
			geometric_distribution<int> num_actions_distribution(0.3);
			int num_actions = num_actions_distribution(generator);
			for (int a_index = 0; a_index < num_actions; a_index++) {
				Action action = problem->random_action();
				problem->perform_action(action);
			}

			vector<ContextLayer> context;
			context.push_back(ContextLayer());

			context.back().scope = solution->scopes[0];
			context.back().node = NULL;

			ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
			context.back().scope_history = root_history;

			// unused
			int exit_depth = -1;
			AbstractNode* exit_node = NULL;

			solution->scopes[0]->activate(
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				root_history);

			if (run_helper.experiments_seen_order.size() == 0) {
				if (!run_helper.exceeded_limit) {
					if (rand()%10 == 0) {
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
				 * - run_helper.experiment_histories.size() == 1
				 */
				Solution* duplicate = new Solution(solution);
				run_helper.experiment_histories.back()->experiment->finalize(duplicate);
				delete run_helper.experiment_histories.back()->experiment;

				double sum_vals = 0.0;
				for (int i_index = 0; i_index < 4000; i_index++) {
					Minesweeper* problem = new Minesweeper();

					RunHelper run_helper;

					geometric_distribution<int> iter_distribution(0.5);
					int num_iters = 1 + iter_distribution(generator);

					for (int i_index = 0; i_index < num_iters; i_index++) {
						geometric_distribution<int> num_actions_distribution(0.3);
						int num_actions = num_actions_distribution(generator);
						for (int a_index = 0; a_index < num_actions; a_index++) {
							Action action = problem->random_action();
							problem->perform_action(action);
						}

						vector<ContextLayer> context;
						context.push_back(ContextLayer());

						context.back().scope = duplicate->scopes[0];
						context.back().node = NULL;

						ScopeHistory* root_history = new ScopeHistory(duplicate->scopes[0]);
						context.back().scope_history = root_history;

						// unused
						int exit_depth = -1;
						AbstractNode* exit_node = NULL;

						duplicate->scopes[0]->activate(
							problem,
							context,
							exit_depth,
							exit_node,
							run_helper,
							root_history);

						delete root_history;
					}

					double target_val;
					if (!run_helper.exceeded_limit) {
						target_val = problem->score_result();
					} else {
						target_val = -1.0;
					}
					sum_vals += target_val;

					delete problem;
				}

				double possible_average_score = sum_vals/4000.0;
				cout << "possible_average_score: " << possible_average_score << endl;

				duplicate->timestamp++;
				duplicate->curr_average_score = possible_average_score;
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

		delete problem;

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
