#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "clean_experiment.h"
#include "globals.h"
#include "outer_experiment.h"
#include "pass_through_experiment.h"
#include "potential_scope_node.h"
#include "retrain_branch_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void create_experiment_helper(vector<int>& scope_context,
							  vector<int>& node_context,
							  vector<AbstractNode*>& possible_nodes,
							  vector<vector<int>>& possible_scope_contexts,
							  vector<vector<int>>& possible_node_contexts,
							  ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	uniform_int_distribution<int> include_branch_distribution(0, 2);
	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			if (h_index == 0 || action_node->action.move != ACTION_NOOP) {
				if (action_node->experiment == NULL) {
					node_context.back() = action_node->id;

					possible_nodes.push_back(action_node);
					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);

					node_context.back() = -1;
				}
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node->id;

			create_experiment_helper(scope_context,
									 node_context,
									 possible_nodes,
									 possible_scope_contexts,
									 possible_node_contexts,
									 scope_node_history->inner_scope_history);

			if (scope_node->experiment == NULL) {
				possible_nodes.push_back(scope_node);
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
			}

			node_context.back() = -1;
		} else {
			if (include_branch_distribution(generator) == 0) {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (!branch_node->branch_is_pass_through) {
					if (branch_node->experiment == NULL) {
						node_context.back() = branch_node->id;

						possible_nodes.push_back(branch_node);
						possible_scope_contexts.push_back(scope_context);
						possible_node_contexts.push_back(node_context);

						node_context.back() = -1;
					}
				}
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void create_experiment(ScopeHistory* root_history) {
	vector<AbstractNode*> possible_nodes;
	vector<vector<int>> possible_scope_contexts;
	vector<vector<int>> possible_node_contexts;

	vector<int> scope_context;
	vector<int> node_context;
	create_experiment_helper(scope_context,
							 node_context,
							 possible_nodes,
							 possible_scope_contexts,
							 possible_node_contexts,
							 root_history);

	uniform_int_distribution<int> possible_distribution(0, (int)possible_nodes.size()-1);
	int rand_index = possible_distribution(generator);

	if (possible_nodes[rand_index]->type == NODE_TYPE_ACTION) {
		uniform_int_distribution<int> context_size_distribution(0, possible_scope_contexts[rand_index].size());
		int context_size = context_size_distribution(generator);

		if (context_size == 0) {
			solution->outer_experiment = new OuterExperiment();
		} else {
			uniform_int_distribution<int> clean_distribution(0, 9);
			uniform_int_distribution<int> pass_through_distribution(0, 3);
			if (clean_distribution(generator) == 0) {
				CleanExperiment* clean_experiment = new CleanExperiment(
					vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
					vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

				ActionNode* action_node = (ActionNode*)possible_nodes[rand_index];
				action_node->experiment = clean_experiment;
			} else {
				if (pass_through_distribution(generator) == 0) {
					PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
						vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
						vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

					ActionNode* action_node = (ActionNode*)possible_nodes[rand_index];
					action_node->experiment = new_pass_through_experiment;
				} else {
					BranchExperiment* new_branch_experiment = new BranchExperiment(
						vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
						vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

					ActionNode* action_node = (ActionNode*)possible_nodes[rand_index];
					action_node->experiment = new_branch_experiment;
				}
			}
		}
	} else if (possible_nodes[rand_index]->type == NODE_TYPE_SCOPE) {
		uniform_int_distribution<int> context_size_distribution(0, possible_scope_contexts[rand_index].size());
		int context_size = context_size_distribution(generator);

		if (context_size == 0) {
			solution->outer_experiment = new OuterExperiment();
		} else {
			uniform_int_distribution<int> clean_distribution(0, 9);
			uniform_int_distribution<int> pass_through_distribution(0, 3);
			if (clean_distribution(generator) == 0) {
				CleanExperiment* clean_experiment = new CleanExperiment(
					vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
					vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

				ScopeNode* scope_node = (ScopeNode*)possible_nodes[rand_index];
				scope_node->experiment = clean_experiment;
			} else {
				if (pass_through_distribution(generator) == 0) {
					PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
						vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
						vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

					ScopeNode* scope_node = (ScopeNode*)possible_nodes[rand_index];
					scope_node->experiment = new_pass_through_experiment;
				} else {
					BranchExperiment* new_branch_experiment = new BranchExperiment(
						vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
						vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

					ScopeNode* scope_node = (ScopeNode*)possible_nodes[rand_index];
					scope_node->experiment = new_branch_experiment;
				}
			}
		}
	} else {
		/**
		 * TODO: add branch and passthrough for BranchNodes too
		 */
		uniform_int_distribution<int> clean_distribution(0, 2);
		if (clean_distribution(generator) == 0) {
			BranchNode* branch_node = (BranchNode*)possible_nodes[rand_index];
			CleanExperiment* clean_experiment = new CleanExperiment(
				branch_node->branch_scope_context,
				branch_node->branch_node_context);
			branch_node->experiment = clean_experiment;
			uniform_int_distribution<int> is_branch_distribution(0, 1);
			branch_node->experiment_is_branch = (is_branch_distribution(generator) == 0);
		} else {
			BranchNode* branch_node = (BranchNode*)possible_nodes[rand_index];
			RetrainBranchExperiment* new_retrain_branch_experiment = new RetrainBranchExperiment(branch_node);
			branch_node->experiment = new_retrain_branch_experiment;
		}
	}
}
