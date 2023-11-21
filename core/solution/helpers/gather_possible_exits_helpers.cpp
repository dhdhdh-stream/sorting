#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void gather_possible_exits_helper(int l_index,
								  vector<pair<int,AbstractNode*>>& possible_exits,
								  vector<ContextLayer>& context,
								  vector<int>& scope_context,
								  vector<int>& node_context,
								  int& exit_depth,
								  AbstractNode*& exit_node) {
	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;
	if (l_index < (int)scope_context.size()-1) {
		gather_possible_exits_helper(l_index+1,
									 possible_exits,
									 context,
									 scope_context,
									 node_context,
									 inner_exit_depth,
									 inner_exit_node);
	}

	if (exit_depth == -1) {
		Scope* scope = context[context.size() - scope_context.size() + l_index].scope;
		
		AbstractNode* starting_node;
		AbstractNode* experiment_node = scope->nodes[node_context[l_index]];
		if (experiment_node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)experiment_node;
			starting_node = action_node->next_node;
		} else {
			ScopeNode* scope_node = (ScopeNode*)experiment_node;
			starting_node = scope_node->next_node;
		}

		scope->random_exit_activate(starting_node,
									scope_context,
									node_context,
									exit_depth,
									exit_node,
									scope_context.size()-1 - l_index,
									possible_exits);
	} else if (exit_depth == 0) {
		Scope* scope = context[context.size() - scope_context.size() + l_index].scope;
		scope->random_exit_activate(inner_exit_node,
									scope_context,
									node_context,
									exit_depth,
									exit_node,
									scope_context.size()-1 - l_index,
									possible_exits);
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}

void gather_possible_exits(vector<pair<int,AbstractNode*>>& possible_exits,
						   vector<ContextLayer>& context,
						   vector<int>& scope_context,
						   vector<int>& node_context) {
	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	gather_possible_exits_helper(0,
								 possible_exits,
								 context,
								 scope_context,
								 node_context,
								 exit_depth,
								 exit_node);
}
