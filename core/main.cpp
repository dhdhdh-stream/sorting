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
#include "run_helper.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"
#include "state_status.h"

using namespace std;

default_random_engine generator;

bool global_debug_flag = false;

Solution* solution;

const int MEASURE_NUM_DATAPOINTS = 1000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	// solution->init();
	ifstream solution_save_file;
	solution_save_file.open("saves/solution.txt");
	solution->load(solution_save_file);
	solution_save_file.close();

	double best_score = -1.0;

	uniform_int_distribution<int> experiment_type_distribution(0, 1);
	while (true) {
		// measure
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < MEASURE_NUM_DATAPOINTS; d_index++) {
			Problem problem;

			RunHelper run_helper;
			run_helper.experiment_history = -1;
			/**
			 * - set to non-NULL to prevent experiment
			 */

			vector<ContextLayer> context;
			context.push_back(ContextLayer());

			context.back().scope_id = 0;
			context.back().node_id = -1;

			ScopeHistory* root_history = new ScopeHistory(solution->root);
			context.back().scope_history = root_history;

			vector<AbstractNode*> starting_nodes{solution->root_starting_node};
			vector<map<int, StateStatus>> starting_input_state_vals;
			vector<map<int, StateStatus>> starting_local_state_vals;

			// unused
			int exit_depth = -1;
			AbstractNode* exit_node = NULL;

			solution->root->activate(starting_nodes,
									 starting_input_state_vals,
									 starting_local_state_vals,
									 problem,
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

			sum_scores += target_val;

			if (!run_helper.exceeded_depth) {
				if (run_helper.max_depth > solution->max_depth) {
					solution->max_depth = run_helper.max_depth;

					if (solution->max_depth < 50) {
						solution->depth_limit = solution->max_depth + 10;
					} else {
						solution->depth_limit = (int)(1.2*(double)solution->max_depth);
					}
				}
			}

			delete root_history;
		}

		solution->average_score = sum_scores / MEASURE_NUM_DATAPOINTS;
		cout << "solution->states.size(): " << solution->states.size() << endl;
		if (solution->average_score > best_score) {
			best_score = solution->average_score;
		}
		// temp
		if (solution->average_score < best_score - 0.1) {
			cout << "ERROR" << endl;
			exit(1);
		}

		while (true) {
			Problem problem;

			RunHelper run_helper;

			vector<ContextLayer> context;
			context.push_back(ContextLayer());

			context.back().scope_id = 0;
			context.back().node_id = -1;

			ScopeHistory* root_history = new ScopeHistory(solution->root);
			context.back().scope_history = root_history;

			vector<AbstractNode*> starting_nodes{solution->root_starting_node};
			vector<map<int, StateStatus>> starting_input_state_vals;
			vector<map<int, StateStatus>> starting_local_state_vals;

			// unused
			int exit_depth = -1;
			AbstractNode* exit_node = NULL;

			root->activate(starting_nodes,
						   starting_input_state_vals,
						   starting_local_state_vals,
						   problem,
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

			bool updated = false;
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
												run_helper.experiment_history);

					if (branch_experiment->state == BRANCH_EXPERIMENT_STATE_FAIL
							|| branch_experiment->state == BRANCH_EXPERIMENT_STATE_SUCCESS) {
						if (branch_experiment->state == BRANCH_EXPERIMENT_STATE_SUCCESS) {
							updated = true;
						}

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
						solution->experiments.erase(branch_experiment);
						delete branch_experiment;
					}
				} else {
					PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)run_helper.experiment_history->experiment;
					pass_through_experiment->backprop(target_val,
													  run_helper.experiment_history);

					if (pass_through_experiment->state == PASS_THROUGH_EXPERIMENT_STATE_FAIL
							|| pass_through_experiment->state == PASS_THROUGH_EXPERIMENT_STATE_SUCCESS) {
						if (pass_through_experiment->state == PASS_THROUGH_EXPERIMENT_STATE_SUCCESS) {
							updated = true;
						}

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
				if (run_helper.experiments_seen.size() == 0) {
					if (experiment_type_distribution(generator) == 0) {
						create_branch_experiment(root_history);
					} else {
						create_pass_through_experiment(root_history);
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

			delete root_history;

			if (updated) {
				break;
			}
		}

		for (set<AbstractExperiment*>::iterator it = solution->experiments.begin();
				it != solution->experiments.end(); it++) {
			AbstractExperiment* experiment = *it;
			Scope* starting_scope = solution->scopes[experiment->scope_context.back()];
			AbstractNode* starting_node = starting_scope->nodes[experiment->node_context.back()];
			if (starting_node->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)starting_node;
				action_node->experiment = NULL;
			} else {
				ScopeNode* scope_node = (ScopeNode*)starting_node;
				scope_node->experiment = NULL;
			}
			delete experiment;
		}
		solution->experiments.clear();
	}

	delete solution;

	cout << "Done" << endl;
}
