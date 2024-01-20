#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "clean_experiment.h"
#include "constants.h"
#include "context_layer.h"
#include "globals.h"
#include "solution_helpers.h"
#include "minesweeper.h"
#include "outer_experiment.h"
#include "pass_through_experiment.h"
#include "potential_scope_node.h"
#include "retrain_branch_experiment.h"
#include "run_helper.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "sorting.h"
#include "state.h"
#include "state_status.h"

using namespace std;

const int NUM_FAILS_BEFORE_INCREASE = 50;

default_random_engine generator;

Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	// int seed = (unsigned)time(NULL);
	int seed = 1705745187;
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->init();
	// solution->load("", "main");

	solution->save("", "main");

	int num_fails = 0;

	#if defined(MDEBUG) && MDEBUG
	int run_index = 0;
	#endif /* MDEBUG */

	while (true) {
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = run_index;
		run_index++;
		#endif /* MDEBUG */

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
				if (run_helper.experiments_seen.size() == 0) {
					if (!run_helper.exceeded_limit) {
						create_experiment(root_history);
					}
				}
			}

			delete root_history;
		}

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_actions);
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

			if (run_helper.experiment_history->experiment->type == EXPERIMENT_TYPE_OUTER) {
				solution->outer_experiment->backprop(target_val,
													 run_helper,
													 (OuterExperimentOverallHistory*)run_helper.experiment_history);

				if (solution->outer_experiment->state == OUTER_EXPERIMENT_STATE_SUCCESS) {
					is_success = true;
					// experiment cleaned in reset()
				} else if (solution->outer_experiment->state == OUTER_EXPERIMENT_STATE_FAIL) {
					is_fail = true;

					delete solution->outer_experiment;
					solution->outer_experiment = NULL;
				}
			} else if (run_helper.experiment_history->experiment->type == EXPERIMENT_TYPE_BRANCH) {
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
					delete branch_experiment;
				}
			} else if (run_helper.experiment_history->experiment->type == EXPERIMENT_TYPE_PASS_THROUGH) {
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
					delete pass_through_experiment;
				}
			} else if (run_helper.experiment_history->experiment->type == EXPERIMENT_TYPE_RETRAIN_BRANCH) {
				RetrainBranchExperiment* retrain_branch_experiment = (RetrainBranchExperiment*)run_helper.experiment_history->experiment;
				retrain_branch_experiment->backprop(target_val,
													run_helper,
													(RetrainBranchExperimentOverallHistory*)run_helper.experiment_history);

				if (retrain_branch_experiment->state == RETRAIN_BRANCH_EXPERIMENT_STATE_SUCCESS) {
					is_success = true;
					// experiment cleaned in reset()
				} else if (retrain_branch_experiment->state == RETRAIN_BRANCH_EXPERIMENT_STATE_FAIL) {
					is_fail = true;

					BranchNode* branch_node = retrain_branch_experiment->branch_node;
					branch_node->experiment = NULL;
					delete retrain_branch_experiment;
				}
			} else {
				CleanExperiment* clean_experiment = (CleanExperiment*)run_helper.experiment_history->experiment;
				clean_experiment->backprop(target_val,
										   run_helper,
										   (CleanExperimentOverallHistory*)run_helper.experiment_history);

				if (clean_experiment->state == CLEAN_EXPERIMENT_STATE_SUCCESS) {
					is_success = true;
					// experiment cleaned in reset()
				} else if (clean_experiment->state == CLEAN_EXPERIMENT_STATE_FAIL) {
					is_fail = true;

					Scope* starting_scope = solution->scopes[clean_experiment->scope_context.back()];
					AbstractNode* starting_node = starting_scope->nodes[clean_experiment->node_context.back()];
					if (starting_node->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)starting_node;
						action_node->experiment = NULL;
					} else if (starting_node->type == NODE_TYPE_SCOPE) {
						ScopeNode* scope_node = (ScopeNode*)starting_node;
						scope_node->experiment = NULL;
					} else {
						BranchNode* branch_node = (BranchNode*)starting_node;
						branch_node->experiment = NULL;
					}
					delete clean_experiment;
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

		delete problem;

		if (is_success) {
			solution->success_reset();

			#if defined(MDEBUG) && MDEBUG
			while (solution->verify_problems.size() > 0) {
				Problem* problem = solution->verify_problems[0];

				RunHelper run_helper;
				run_helper.verify_key = solution->verify_key;

				run_helper.starting_run_seed = solution->verify_seeds[0];
				run_helper.curr_run_seed = solution->verify_seeds[0];
				solution->verify_seeds.erase(solution->verify_seeds.begin());

				vector<ContextLayer> context;
				context.push_back(ContextLayer());

				context.back().scope = solution->root;
				context.back().node = NULL;

				// unused
				int exit_depth = -1;
				AbstractNode* exit_node = NULL;

				solution->root->verify_activate(problem,
												context,
												exit_depth,
												exit_node,
												run_helper);

				delete solution->verify_problems[0];
				solution->verify_problems.erase(solution->verify_problems.begin());
			}
			solution->clear_verify();
			#endif /* MDEBUG */

			num_fails = 0;

			solution->id = (unsigned)time(NULL);
			solution->save("", "main");

			ofstream display_file;
			display_file.open("../display.txt");
			solution->save_for_display(display_file);
			display_file.close();

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
		}

		#if defined(MDEBUG) && MDEBUG
		if (run_index%1000 == 0) {
			delete solution;
			solution = new Solution();
			solution->load("", "main");
		}
		#endif /* MDEBUG */
	}

	delete solution;

	cout << "Done" << endl;
}
