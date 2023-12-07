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
#include "minesweeper.h"
#include "outer_experiment.h"
#include "pass_through_experiment.h"
#include "potential_scope_node.h"
#include "run_helper.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "sorting.h"
#include "state.h"
#include "state_status.h"

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

	#if defined(MDEBUG) && MDEBUG
	int run_index = 0;
	#endif /* MDEBUG */

	int iter_index = 0;
	uniform_int_distribution<int> outer_distribution(0, 7);
	while (true) {
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = run_index;
		run_index++;
		#endif /* MDEBUG */

		bool is_success = false;
		bool is_fail = false;
		if (outer_distribution(generator) == 0) {
			solution->outer_experiment->activate(problem,
												 run_helper);

			double target_val;
			if (!run_helper.exceeded_limit) {
				target_val = problem->score_result(run_helper.num_actions);
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
									 0,
									 root_history);

			if (root_history->inner_pass_through_experiment != NULL) {
				root_history->inner_pass_through_experiment->parent_scope_end_activate(
					context,
					run_helper,
					root_history);
			}

			double target_val;
			if (!run_helper.exceeded_limit) {
				target_val = problem->score_result(run_helper.num_actions);
			} else {
				target_val = -1.0;
			}

			if (run_helper.experiment_history == NULL) {
				if (run_helper.experiments_seen.size() == 0) {
					if (!run_helper.exceeded_limit) {
						create_experiment(root_history);
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

						Scope* starting_scope = solution->scopes[branch_experiment->scope_context.back()];
						AbstractNode* starting_node = starting_scope->nodes[branch_experiment->node_context.back()];
						if (starting_node->type == NODE_TYPE_ACTION) {
							ActionNode* action_node = (ActionNode*)starting_node;
							action_node->experiment = NULL;
						} else {
							ScopeNode* scope_node = (ScopeNode*)starting_node;
							scope_node->experiment = NULL;
						}
						delete branch_experiment;
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
						delete branch_experiment;
					}
				} else {
					PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)run_helper.experiment_history->experiment;
					pass_through_experiment->backprop(target_val,
													  run_helper,
													  (PassThroughExperimentOverallHistory*)run_helper.experiment_history);

					if (pass_through_experiment->state == PASS_THROUGH_EXPERIMENT_STATE_SUCCESS) {
						is_success = true;

						Scope* starting_scope = solution->scopes[pass_through_experiment->scope_context.back()];
						AbstractNode* starting_node = starting_scope->nodes[pass_through_experiment->node_context.back()];
						if (starting_node->type == NODE_TYPE_ACTION) {
							ActionNode* action_node = (ActionNode*)starting_node;
							action_node->experiment = NULL;
						} else {
							ScopeNode* scope_node = (ScopeNode*)starting_node;
							scope_node->experiment = NULL;
						}
						delete pass_through_experiment;
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

		delete problem;

		if (is_success) {
			ifstream solution_save_file;
			solution_save_file.open(path + "saves/main/solution.txt");
			string id_line;
			getline(solution_save_file, id_line);
			int curr_id = stoi(id_line);
			solution_save_file.close();

			if (curr_id > solution->id) {
				delete solution;

				solution = new Solution();
				solution->load(path, "main");

				cout << "updated from main" << endl;
			} else {
				solution->success_reset();

				solution->id = (unsigned)time(NULL);
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
			iter_index++;
			if (iter_index%20000 == 0) {
				ifstream solution_save_file;
				solution_save_file.open(path + "saves/main/solution.txt");
				string id_line;
				getline(solution_save_file, id_line);
				int curr_id = stoi(id_line);
				solution_save_file.close();

				if (curr_id > solution->id) {
					delete solution;

					solution = new Solution();
					solution->load(path, "main");

					cout << "updated from main" << endl;

					num_fails = 0;

					solution->curr_num_datapoints = STARTING_NUM_DATAPOINTS;
				}
			}
		}
	}

	delete solution;

	cout << "Done" << endl;
}
