#include "solution_helpers.h"

#include "action_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void count_actions_helper(int& num_actions,
						  ScopeHistory* scope_history) {
	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			if (action_node->action.move != ACTION_NOOP) {
				num_actions++;
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			count_actions_helper(num_actions,
								 scope_node_history->scope_history);
		}
	}
}

int count_actions(ScopeHistory* scope_history) {
	int num_actions = 0;
	count_actions_helper(num_actions,
						 scope_history);
	return num_actions;
}
