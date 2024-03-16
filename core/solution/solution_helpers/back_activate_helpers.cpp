#include "solution_helpers.h"

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
							vector<vector<Scope*>>& possible_scope_contexts,
							vector<vector<AbstractNode*>>& possible_node_contexts,
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

					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);

					node_context.back() = NULL;
				}
			}

			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				uniform_int_distribution<int> distribution(0, 2);
				if (distribution(generator) != 0) {
					gather_possible_helper(scope_context,
										   node_context,
										   possible_scope_contexts,
										   possible_node_contexts,
										   scope_node_history->scope_history);
				}

				node_context.back() = NULL;
			}

			break;
		case NODE_TYPE_BRANCH:
			{
				node_context.back() = node_history->node;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);

				node_context.back() = NULL;
			}

			break;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void input_vals_helper(int curr_depth,
					   int max_depth,
					   vector<Scope*>& scope_context,
					   vector<AbstractNode*>& node_context,
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
										   input_vals,
										   action_node_history);
			}

			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				if (curr_depth+1 < max_depth) {
					input_vals_helper(curr_depth+1,
									  max_depth,
									  scope_context,
									  node_context,
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
										   input_vals,
										   branch_node_history);
			}

			break;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}
