#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "potential_scope_node.h"
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
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				if (action_node->experiment == NULL) {
					node_context.back() = action_node->id;

					possible_nodes.push_back(action_node);
					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);

					node_context.back() = -1;
				}
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
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
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void create_branch_experiment(ScopeHistory* root_history) {
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

	uniform_int_distribution<int> next_distribution(0, 1);
	int context_size = 1;
	while (true) {
		if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
			context_size++;
		} else {
			break;
		}
	}

	BranchExperiment* new_branch_experiment = new BranchExperiment(
		vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
		vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

	if (possible_nodes[rand_index]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)possible_nodes[rand_index];
		action_node->experiment = new_branch_experiment;
	} else {
		ScopeNode* scope_node = (ScopeNode*)possible_nodes[rand_index];
		scope_node->experiment = new_branch_experiment;
	}
}

void create_pass_through_experiment(ScopeHistory* root_history) {
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

	uniform_int_distribution<int> next_distribution(0, 1);
	int context_size = 1;
	while (true) {
		if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
			context_size++;
		} else {
			break;
		}
	}

	PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
		vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
		vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

	if (possible_nodes[rand_index]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)possible_nodes[rand_index];
		action_node->experiment = new_pass_through_experiment;
	} else {
		ScopeNode* scope_node = (ScopeNode*)possible_nodes[rand_index];
		scope_node->experiment = new_pass_through_experiment;
	}
}
