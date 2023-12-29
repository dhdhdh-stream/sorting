#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "potential_scope_node.h"
#include "retrain_branch_experiment.h"
#include "retrain_loop_experiment.h"
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

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			AbstractNodeHistory* node_history = scope_history->node_histories[i_index][h_index];
			if (node_history->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				if (action_node->experiment == NULL) {
					node_context.back() = action_node->id;

					possible_nodes.push_back(action_node);
					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);

					node_context.back() = -1;
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

					/**
					 * - simply add twice and randomly create between retrain loop and normal
					 */
					if (scope_node->is_loop) {
						possible_nodes.push_back(scope_node);
						possible_scope_contexts.push_back(scope_context);
						possible_node_contexts.push_back(node_context);
					}
				}

				node_context.back() = -1;
			} else {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
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
		uniform_int_distribution<int> next_distribution(0, 1);
		int context_size = 1;
		while (true) {
			if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
				context_size++;
			} else {
				break;
			}
		}

		uniform_int_distribution<int> type_distribution(0, 1);
		if (type_distribution(generator) == 0) {
			BranchExperiment* new_branch_experiment = new BranchExperiment(
				vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
				vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

			ActionNode* action_node = (ActionNode*)possible_nodes[rand_index];
			action_node->experiment = new_branch_experiment;
		} else {
			PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
				vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
				vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

			ActionNode* action_node = (ActionNode*)possible_nodes[rand_index];
			action_node->experiment = new_pass_through_experiment;
		}
	} else if (possible_nodes[rand_index]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)possible_nodes[rand_index];
		uniform_int_distribution<int> retrain_loop_distribution(0, 1);
		if (scope_node->is_loop && retrain_loop_distribution(generator) == 0) {
			RetrainLoopExperiment* new_retrain_loop_experiment = new RetrainLoopExperiment(scope_node);
			scope_node->experiment = new_retrain_loop_experiment;
		} else {
			uniform_int_distribution<int> next_distribution(0, 1);
			int context_size = 1;
			while (true) {
				if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
					context_size++;
				} else {
					break;
				}
			}

			uniform_int_distribution<int> type_distribution(0, 1);
			if (type_distribution(generator) == 0) {
				BranchExperiment* new_branch_experiment = new BranchExperiment(
					vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
					vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

				scope_node->experiment = new_branch_experiment;
			} else {
				PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
					vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
					vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

				scope_node->experiment = new_pass_through_experiment;
			}
		}
	} else {
		BranchNode* branch_node = (BranchNode*)possible_nodes[rand_index];
		RetrainBranchExperiment* new_retrain_branch_experiment = new RetrainBranchExperiment(branch_node);
		branch_node->experiment = new_retrain_branch_experiment;
	}
}
