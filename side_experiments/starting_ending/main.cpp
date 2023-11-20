#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "context_layer.h"
#include "globals.h"
#include "helpers.h"
#include "outer_experiment.h"
#include "pass_through_experiment.h"
#include "run_helper.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "state_status.h"

using namespace std;

const int NUM_FAILS_BEFORE_INCREASE = 50;

default_random_engine generator;

bool global_debug_flag = false;

Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->init();
	// ifstream solution_save_file;
	// solution_save_file.open("saves/solution.txt");
	// solution->load(solution_save_file);
	// solution_save_file.close();

	int num_fails = 0;

	uniform_int_distribution<int> outer_distribution(0, 9);
	uniform_int_distribution<int> experiment_type_distribution(0, 1);
	while (true) {
		while (true) {
			Problem problem;

			RunHelper run_helper;

			bool is_success = false;
			bool is_fail = false;
			if (outer_distribution(generator) == 0) {
				solution->outer_experiment->activate(problem,
													 run_helper);

				double target_val;
				if (!run_helper.exceeded_depth) {
					target_val = problem.score_result();
				} else {
					target_val = -1.0;
				}

				solution->outer_experiment->backprop(target_val,
													 run_helper);

				if (solution->outer_experiment->state == OUTER_EXPERIMENT_STATE_SUCCESS) {
					is_success = true;

					// outer_experiment cleaned in reset()
				}
			} else {
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

				double target_val;
				if (!run_helper.exceeded_depth) {
					target_val = problem.score_result();
				} else {
					target_val = -1.0;
				}

				if (run_helper.experiment_history == NULL) {
					if (run_helper.experiments_seen.size() == 0) {
						if (experiment_type_distribution(generator) == 0) {
							create_branch_experiment(root_history);
						} else {
							create_pass_through_experiment(root_history);
						}
					}
				}

				delete root_history;

				if (run_helper.experiment_history != NULL) {
					for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
						AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
						experiment->average_remaining_experiments_from_start =
							0.9 * experiment->average_remaining_experiments_from_start
							+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
								+ run_helper.experiment_history->experiment->average_remaining_experiments_from_start);
					}

					if (run_helper.experiment_history->experiment->type == EXPERIMENT_TYPE_BRANCH) {
						BranchExperiment* branch_experiment = (BranchExperiment*)run_helper.experiment_history->experiment;
						branch_experiment->backprop(target_val,
													run_helper,
													(BranchExperimentOverallHistory*)run_helper.experiment_history);

						if (branch_experiment->state == BRANCH_EXPERIMENT_STATE_SUCCESS) {
							is_success = true;

							map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
							map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
							branch_experiment->finalize(input_scope_depths_mappings,
														output_scope_depths_mappings);

							// experiment cleaned in reset()
						} else if (branch_experiment->state == BRANCH_EXPERIMENT_STATE_FAIL) {
							is_fail = true;

							Scope* starting_scope = solution->scopes[branch_experiment->scope_context.back()];
							AbstractNode* starting_node = starting_scope->nodes[branch_experiment->node_context.back()];
							if (starting_node->type == NODE_TYPE_ACTION) {
								ActionNode* action_node = (ActionNode*)starting_node;
								action_node->experiment = NULL;
							} else {
								ScopeNode* scope_node = (ScopeNode*)starting_node;
								scope_node->experiment = NULL;
							}
							solution->experiments.erase(branch_experiment);
							delete branch_experiment;
						}
					} else {
						PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)run_helper.experiment_history->experiment;
						pass_through_experiment->backprop(target_val,
														  run_helper,
														  (PassThroughExperimentOverallHistory*)run_helper.experiment_history);

						if (pass_through_experiment->state == PASS_THROUGH_EXPERIMENT_STATE_SUCCESS) {
							is_success = true;

							// experiment cleaned in reset()
						} else if (pass_through_experiment->state == PASS_THROUGH_EXPERIMENT_STATE_FAIL) {
							is_fail = true;

							Scope* starting_scope = solution->scopes[pass_through_experiment->scope_context.back()];
							AbstractNode* starting_node = starting_scope->nodes[pass_through_experiment->node_context.back()];
							if (starting_node->type == NODE_TYPE_ACTION) {
								ActionNode* action_node = (ActionNode*)starting_node;
								action_node->experiment = NULL;
							} else {
								ScopeNode* scope_node = (ScopeNode*)starting_node;
								scope_node->experiment = NULL;
							}
							solution->experiments.erase(pass_through_experiment);
							delete pass_through_experiment;
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

			if (is_success) {
				num_fails = 0;
				solution->success_reset();

				ofstream solution_save_file;
				solution_save_file.open("saves/solution.txt");
				solution->save(solution_save_file);
				solution_save_file.close();

				ofstream display_file;
				display_file.open("../display.txt");
				solution->save_for_display(display_file);
				display_file.close();

				solution->curr_num_datapoints = STARTING_NUM_DATAPOINTS;
				break;
			} else if (is_fail) {
				num_fails++;
				cout << "num_fails: " << num_fails << endl << endl;
				if (num_fails > NUM_FAILS_BEFORE_INCREASE) {
					cout << "fail_reset" << endl << endl;

					num_fails = 0;
					solution->fail_reset();

					solution->curr_num_datapoints *= 2;
					break;
				}
			}
		}
	}

	delete solution;

	cout << "Done" << endl;
}
