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

	auto start_time = chrono::high_resolution_clock::now();

	while (solution->timestamp < EXPLORE_ITERS) {
		Solution* best_solution = NULL;

		int improvement_iter = 0;

		while (true) {
			auto curr_time = chrono::high_resolution_clock::now();
			auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
			if (time_diff.count() >= 20) {
				start_time = curr_time;

				cout << "alive" << endl;
			}

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			vector<ContextLayer> context;
			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->experiment_activate(
					problem,
					context,
					run_helper,
					scope_history);

			double target_val = problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;

			if (!run_helper.has_explore) {
				update_scores(scope_history,
							  target_val);
			}

			if (run_helper.experiments_seen_order.size() == 0) {
				create_experiment(scope_history);
			}

			delete scope_history;
			delete problem;

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
					run_helper);
				if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
					run_helper.experiment_history->experiment->finalize(NULL);
					delete run_helper.experiment_history->experiment;
				} else if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
					Solution* duplicate = new Solution(solution);

					int last_updated_scope_id = run_helper.experiment_history->experiment->scope_context->id;

					run_helper.experiment_history->experiment->finalize(duplicate);
					delete run_helper.experiment_history->experiment;

					Scope* experiment_scope = duplicate->scopes[last_updated_scope_id];
					clean_scope(experiment_scope,
								duplicate);
					duplicate->clean_scopes();

					double sum_score = 0.0;
					double sum_true_score = 0.0;
					for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
						auto curr_time = chrono::high_resolution_clock::now();
						auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
						if (time_diff.count() >= 20) {
							start_time = curr_time;

							cout << "alive" << endl;
						}

						Problem* problem = problem_type->get_problem();

						RunHelper run_helper;

						vector<ContextLayer> context;
						ScopeHistory* scope_history = new ScopeHistory(duplicate->scopes[0]);
						duplicate->scopes[0]->measure_activate(
							problem,
							context,
							run_helper,
							scope_history);
						delete scope_history;

						double target_val = problem->score_result();
						sum_score += target_val - 0.05 * run_helper.num_actions * solution->curr_time_penalty
							- run_helper.num_analyze * solution->curr_time_penalty;
						sum_true_score += target_val;

						delete problem;
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
					if (duplicate->curr_true_score > duplicate->best_true_score) {
						duplicate->best_true_score = duplicate->curr_true_score;
						duplicate->best_true_score_timestamp = duplicate->timestamp;
					}
					if (duplicate->best_true_score_timestamp < duplicate->timestamp
							&& (duplicate->timestamp - duplicate->best_true_score_timestamp)
								% DECREASE_TIME_PENALTY_ITER == 0) {
						duplicate->curr_time_penalty *= 0.8;
					}

					if (best_solution == NULL
							|| duplicate->curr_score > best_solution->curr_score) {
						if (best_solution != NULL) {
							delete best_solution;
						}

						best_solution = duplicate;
					} else {
						delete duplicate;
					}

					improvement_iter++;
					if (solution->was_commit) {
						if (improvement_iter >= COMMIT_IMPROVEMENTS_PER_ITER) {
							break;
						}
					} else {
						if (improvement_iter >= IMPROVEMENTS_PER_ITER) {
							break;
						}
					}
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

		if (solution->was_commit) {
			if (best_solution->curr_score < solution->curr_score) {
				switch (solution->commit_start_node->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)solution->commit_start_node;
						action_node->next_node = solution->commit_exit_node;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)solution->commit_start_node;
						scope_node->next_node = solution->commit_exit_node;
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)solution->commit_start_node;
						if (solution->commit_is_branch) {
							branch_node->branch_next_node = solution->commit_exit_node;
						} else {
							branch_node->original_next_node = solution->commit_exit_node;
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)solution->commit_start_node;
						obs_node->next_node = solution->commit_exit_node;
					}
					break;
				}

				clean_scope(solution->commit_scope,
							solution);

				solution->timestamp++;
				solution->was_commit = false;
			} else {
				delete solution;
				solution = best_solution;
			}
		} else {
			delete solution;
			solution = best_solution;
		}

		if (solution->timestamp % COMMIT_ITERS == 0
				&& solution->timestamp != EXPLORE_ITERS) {
			while (true) {
				auto curr_time = chrono::high_resolution_clock::now();
				auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
				if (time_diff.count() >= 20) {
					start_time = curr_time;

					cout << "alive" << endl;
				}

				Problem* problem = problem_type->get_problem();

				RunHelper run_helper;

				vector<ContextLayer> context;
				ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
				solution->scopes[0]->experiment_activate(
						problem,
						context,
						run_helper,
						scope_history);

				double target_val = problem->score_result();
				target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
				target_val -= run_helper.num_analyze * solution->curr_time_penalty;

				if (run_helper.experiments_seen_order.size() == 0) {
					create_commit_experiment(scope_history);
				}

				delete scope_history;
				delete problem;

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
						run_helper);
					if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
						run_helper.experiment_history->experiment->finalize(NULL);
						delete run_helper.experiment_history->experiment;
					} else if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
						int last_updated_scope_id = run_helper.experiment_history->experiment->scope_context->id;

						run_helper.experiment_history->experiment->finalize(solution);
						delete run_helper.experiment_history->experiment;

						Scope* experiment_scope = solution->scopes[last_updated_scope_id];
						clean_scope(experiment_scope,
									solution);
						solution->clean_scopes();

						solution->clear_experiments();

						break;
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
		}

		solution->save(path, filename);
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
