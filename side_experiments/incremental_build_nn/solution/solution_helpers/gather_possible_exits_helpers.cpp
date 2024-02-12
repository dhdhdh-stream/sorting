#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void gather_possible_exits_helper(int l_index,
								  vector<pair<int,AbstractNode*>>& possible_exits,
								  vector<ContextLayer>& context,
								  vector<Scope*>& scope_context,
								  vector<AbstractNode*>& node_context,
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

	if (inner_exit_depth == -1) {
		Scope* scope = context[context.size() - scope_context.size() + l_index].scope;
		
		AbstractNode* starting_node;
		AbstractNode* experiment_node = node_context[l_index];
		if (experiment_node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)experiment_node;
			starting_node = action_node->next_node;
		} else if (experiment_node->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)experiment_node;
			starting_node = scope_node->next_node;
		} else {
			BranchNode* branch_node = (BranchNode*)experiment_node;
			if (branch_node->experiment_is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}

		scope->random_exit_activate(starting_node,
									scope_context,
									node_context,
									exit_depth,
									exit_node,
									scope_context.size()-1 - l_index,
									possible_exits);
	} else if (inner_exit_depth == 0) {
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
						   vector<Scope*>& scope_context,
						   vector<AbstractNode*>& node_context) {
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

void parent_pass_through_gather_possible_exits_helper(
		int l_index,
		vector<pair<int,AbstractNode*>>& possible_exits,
		vector<ContextLayer>& context,
		vector<Scope*>& scope_context,
		vector<AbstractNode*>& node_context,
		int parent_exit_depth,
		AbstractNode* parent_exit_node,
		int& exit_depth,
		AbstractNode*& exit_node) {
	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;
	if (l_index < (int)scope_context.size()-1 - parent_exit_depth) {
		parent_pass_through_gather_possible_exits_helper(
			l_index+1,
			possible_exits,
			context,
			scope_context,
			node_context,
			parent_exit_depth,
			parent_exit_node,
			inner_exit_depth,
			inner_exit_node);
	}

	if (inner_exit_depth == -1) {
		Scope* scope = context[context.size() - scope_context.size() + l_index].scope;

		AbstractNode* starting_node;
		if (l_index == (int)scope_context.size()-1 - parent_exit_depth) {
			starting_node = parent_exit_node;
		} else {
			ScopeNode* scope_node = (ScopeNode*)node_context[l_index];
			starting_node = scope_node->next_node;
		}

		scope->random_exit_activate(starting_node,
									scope_context,
									node_context,
									exit_depth,
									exit_node,
									scope_context.size()-1 - l_index,
									possible_exits);
	} else if (inner_exit_depth == 0) {
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

void parent_pass_through_gather_possible_exits(vector<pair<int,AbstractNode*>>& possible_exits,
											   vector<ContextLayer>& context,
											   vector<Scope*>& scope_context,
											   vector<AbstractNode*>& node_context,
											   int parent_exit_depth,
											   AbstractNode* parent_exit_node) {
	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	parent_pass_through_gather_possible_exits_helper(0,
													 possible_exits,
													 context,
													 scope_context,
													 node_context,
													 parent_exit_depth,
													 parent_exit_node,
													 exit_depth,
													 exit_node);
}
