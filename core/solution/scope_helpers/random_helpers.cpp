#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "exit_node.h"
#include "globals.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_random_activate_helper(AbstractNode*& curr_node,
								 vector<Scope*>& scope_context,
								 vector<AbstractNode*>& node_context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 int& random_curr_depth,
								 bool& random_exceeded_limit,
								 vector<vector<Scope*>>& possible_scope_contexts,
								 vector<vector<AbstractNode*>>& possible_node_contexts) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* node = (ActionNode*)curr_node;

		node_context.back() = node;

		possible_scope_contexts.push_back(scope_context);
		possible_node_contexts.push_back(node_context);

		node_context.back() = NULL;

		curr_node = node->next_node;
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* node = (ScopeNode*)curr_node;

		int inner_exit_depth = -1;
		AbstractNode* inner_exit_node = NULL;

		node->random_activate(scope_context,
							  node_context,
							  inner_exit_depth,
							  inner_exit_node,
							  random_curr_depth,
							  random_exceeded_limit,
							  possible_scope_contexts,
							  possible_node_contexts);

		if (inner_exit_depth == -1) {
			curr_node = node->next_node;
		} else if (inner_exit_depth == 0) {
			curr_node = inner_exit_node;
		} else {
			exit_depth = inner_exit_depth-1;
			exit_node = inner_exit_node;
		}
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;

		bool is_branch;
		node->random_activate(is_branch,
							  scope_context,
							  node_context,
							  possible_scope_contexts,
							  possible_node_contexts);

		if (is_branch) {
			curr_node = node->branch_next_node;
		} else {
			curr_node = node->original_next_node;
		}
	} else {
		ExitNode* node = (ExitNode*)curr_node;

		node_context.back() = node;

		possible_scope_contexts.push_back(scope_context);
		possible_node_contexts.push_back(node_context);

		node_context.back() = NULL;

		exit_depth = node->exit_depth-1;
		exit_node = node->next_node;
	}
}

void Scope::random_activate(vector<Scope*>& scope_context,
							vector<AbstractNode*>& node_context,
							int& exit_depth,
							AbstractNode*& exit_node,
							int& random_curr_depth,
							bool& random_exceeded_limit,
							vector<vector<Scope*>>& possible_scope_contexts,
							vector<vector<AbstractNode*>>& possible_node_contexts) {
	if (random_curr_depth > solution->depth_limit) {
		random_exceeded_limit = true;
		return;
	}
	random_curr_depth++;

	AbstractNode* curr_node = this->starting_node;
	while (true) {
		if (random_exceeded_limit
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		node_random_activate_helper(curr_node,
									scope_context,
									node_context,
									exit_depth,
									exit_node,
									random_curr_depth,
									random_exceeded_limit,
									possible_scope_contexts,
									possible_node_contexts);
	}

	if (exit_depth == -1) {
		possible_scope_contexts.push_back(scope_context);
		possible_node_contexts.push_back(node_context);
	}

	random_curr_depth--;
}

void node_random_exit_activate_helper(AbstractNode*& curr_node,
									  vector<Scope*>& scope_context,
									  vector<AbstractNode*>& node_context,
									  int& exit_depth,
									  AbstractNode*& exit_node,
									  int curr_depth,
									  vector<pair<int,AbstractNode*>>& possible_exits) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* node = (ActionNode*)curr_node;

		possible_exits.push_back({curr_depth, curr_node});

		curr_node = node->next_node;
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* node = (ScopeNode*)curr_node;

		possible_exits.push_back({curr_depth, curr_node});

		curr_node = node->next_node;
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;

		bool is_branch;
		node->random_exit_activate(is_branch,
								   scope_context,
								   node_context,
								   curr_depth,
								   possible_exits);

		if (is_branch) {
			curr_node = node->branch_next_node;
		} else {
			curr_node = node->original_next_node;
		}
	} else {
		ExitNode* node = (ExitNode*)curr_node;

		possible_exits.push_back({curr_depth, curr_node});

		exit_depth = node->exit_depth-1;
		exit_node = node->next_node;
	}
}

void Scope::random_exit_activate(AbstractNode* starting_node,
								 vector<Scope*>& scope_context,
								 vector<AbstractNode*>& node_context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 int curr_depth,
								 vector<pair<int,AbstractNode*>>& possible_exits) {
	AbstractNode* curr_node = starting_node;
	while (true) {
		if (exit_depth != -1 || curr_node == NULL) {
			break;
		}

		node_random_exit_activate_helper(curr_node,
										 scope_context,
										 node_context,
										 exit_depth,
										 exit_node,
										 curr_depth,
										 possible_exits);
	}

	if (exit_depth == -1) {
		possible_exits.push_back({curr_depth, NULL});
	}
}
