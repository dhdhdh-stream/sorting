#include "solution_helpers.h"

using namespace std;

void input_vals_helper(vector<Scope*>& scope_context,
					   vector<AbstractNode*>& node_context,
					   vector<double>& input_vals,
					   ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			action_node->back_activate(scope_context,
									   node_context,
									   input_vals,
									   action_node_history);
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			input_vals_helper(scope_context,
							  node_context,
							  input_vals,
							  scope_node_history->inner_scope_history);

			node_context.back() = NULL;
		} else {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;
			BranchNode* branch_node = (BranchNode*)branch_node_history->node;
			branch_node->back_activate(scope_context,
									   node_context,
									   input_vals,
									   branch_node_history);
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}
