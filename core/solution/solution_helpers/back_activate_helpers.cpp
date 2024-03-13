#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void gather_possible_helper(vector<Scope*>& scope_context,
							vector<AbstractNode*>& node_context,
							bool on_path,
							int path_depth,
							vector<int>& context_match_indexes,
							vector<vector<Scope*>>& possible_scope_contexts,
							vector<vector<AbstractNode*>>& possible_node_contexts,
							vector<int>& possible_strict_root_indexes,
							ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		switch (node_history->node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				if (h_index == 0 || action_node->action.move != ACTION_NOOP) {
					node_context.back() = action_node;

					int root_index;
					for (int l_index = 0; l_index < (int)context_match_indexes.size(); l_index++) {
						if (path_depth >= context_match_indexes[l_index]-context_match_indexes[0]) {
							root_index = l_index;
						} else {
							break;
						}
					}

					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);
					possible_strict_root_indexes.push_back(root_index);

					node_context.back() = NULL;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				if (on_path
						&& h_index == (int)scope_history->node_histories.size()-1
						&& path_depth < context_match_indexes.back()-1) {
					gather_possible_helper(scope_context,
										   node_context,
										   true,
										   path_depth+1,
										   context_match_indexes,
										   possible_scope_contexts,
										   possible_node_contexts,
										   possible_strict_root_indexes,
										   scope_node_history->scope_history);
				} else {
					/**
					 * - focus on inputs along the path
					 */
					uniform_int_distribution<int> inner_distribution(0, 2);
					if (inner_distribution(generator) != 0) {
						gather_possible_helper(scope_context,
											   node_context,
											   false,
											   path_depth,
											   context_match_indexes,
											   possible_scope_contexts,
											   possible_node_contexts,
											   possible_strict_root_indexes,
											   scope_node_history->scope_history);
					}
				}

				node_context.back() = NULL;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				node_context.back() = node_history->node;

				int root_index;
				for (int l_index = 0; l_index < (int)context_match_indexes.size(); l_index++) {
					if (path_depth >= context_match_indexes[l_index]-context_match_indexes[0]) {
						root_index = l_index;
					} else {
						break;
					}
				}

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_strict_root_indexes.push_back(root_index);

				node_context.back() = NULL;
			}
			break;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void input_vals_helper(vector<Scope*>& scope_context,
					   vector<AbstractNode*>& node_context,
					   bool on_path,
					   int path_depth,
					   vector<int>& context_match_indexes,
					   vector<double>& input_vals,
					   ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		switch (node_history->node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->back_activate(scope_context,
										   node_context,
										   path_depth,
										   context_match_indexes,
										   input_vals,
										   action_node_history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				if (on_path
						&& h_index == (int)scope_history->node_histories.size()-1
						&& path_depth < context_match_indexes.back()-1) {
					input_vals_helper(scope_context,
									  node_context,
									  true,
									  path_depth+1,
									  context_match_indexes,
									  input_vals,
									  scope_node_history->scope_history);
				} else {
					input_vals_helper(scope_context,
									  node_context,
									  false,
									  path_depth,
									  context_match_indexes,
									  input_vals,
									  scope_node_history->scope_history);
				}

				node_context.back() = NULL;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				branch_node->back_activate(scope_context,
										   node_context,
										   path_depth,
										   context_match_indexes,
										   input_vals,
										   branch_node_history);
			}
			break;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}
