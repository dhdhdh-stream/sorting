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
								 int& random_throw_id,
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
		node->random_activate(curr_node,
							  scope_context,
							  node_context,
							  exit_depth,
							  exit_node,
							  random_curr_depth,
							  random_throw_id,
							  random_exceeded_limit,
							  possible_scope_contexts,
							  possible_node_contexts);
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;
		node->random_activate(curr_node,
							  scope_context,
							  node_context,
							  possible_scope_contexts,
							  possible_node_contexts);
	} else {
		ExitNode* node = (ExitNode*)curr_node;

		node_context.back() = node;

		possible_scope_contexts.push_back(scope_context);
		possible_node_contexts.push_back(node_context);

		node_context.back() = NULL;

		if (node->throw_id != -1) {
			random_throw_id = node->throw_id;
		} else {
			exit_depth = node->exit_depth-1;
			exit_node = node->next_node;
		}
	}
}

void Scope::random_activate(vector<Scope*>& scope_context,
							vector<AbstractNode*>& node_context,
							int& exit_depth,
							AbstractNode*& exit_node,
							int& random_curr_depth,
							int& random_throw_id,
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
				|| random_throw_id != -1
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
									random_throw_id,
									random_exceeded_limit,
									possible_scope_contexts,
									possible_node_contexts);
	}

	random_curr_depth--;
}

void node_random_exit_activate_helper(AbstractNode*& curr_node,
									  vector<Scope*>& scope_context,
									  vector<AbstractNode*>& node_context,
									  int& exit_depth,
									  AbstractNode*& exit_node,
									  int& random_curr_depth,
									  int& random_throw_id,
									  bool& random_exceeded_limit,
									  int curr_depth,
									  vector<pair<int,AbstractNode*>>& possible_exits) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* node = (ActionNode*)curr_node;

		possible_exits.push_back({curr_depth, curr_node});

		curr_node = node->next_node;
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* node = (ScopeNode*)curr_node;
		node->random_exit_activate(curr_node,
								   scope_context,
								   node_context,
								   exit_depth,
								   exit_node,
								   random_curr_depth,
								   random_throw_id,
								   random_exceeded_limit,
								   curr_depth,
								   possible_exits);
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;
		node->random_exit_activate(curr_node,
								   scope_context,
								   node_context,
								   curr_depth,
								   possible_exits);
	} else {
		ExitNode* node = (ExitNode*)curr_node;

		possible_exits.push_back({curr_depth, curr_node});

		if (node->throw_id != -1) {
			random_throw_id = node->throw_id;
		} else {
			exit_depth = node->exit_depth-1;
			exit_node = node->next_node;
		}
	}
}

void Scope::random_exit_activate(AbstractNode* starting_node,
								 vector<Scope*>& scope_context,
								 vector<AbstractNode*>& node_context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 int& random_curr_depth,
								 int& random_throw_id,
								 bool& random_exceeded_limit,
								 int curr_depth,
								 vector<pair<int,AbstractNode*>>& possible_exits) {
	/**
	 * - don't need to increment random_curr_depth
	 */

	AbstractNode* curr_node = starting_node;
	while (true) {
		if (random_exceeded_limit
				|| random_throw_id != -1
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		node_random_exit_activate_helper(curr_node,
										 scope_context,
										 node_context,
										 exit_depth,
										 exit_node,
										 random_curr_depth,
										 random_throw_id,
										 random_exceeded_limit,
										 curr_depth,
										 possible_exits);
	}

	if (!random_exceeded_limit
			&& random_throw_id == -1
			&& exit_depth == -1) {
		possible_exits.push_back({curr_depth, NULL});
	}

	random_curr_depth--;
}

void node_inner_random_exit_activate_helper(AbstractNode*& curr_node,
											vector<Scope*>& scope_context,
											vector<AbstractNode*>& node_context,
											int& exit_depth,
											AbstractNode*& exit_node,
											int& random_curr_depth,
											int& random_throw_id,
											bool& random_exceeded_limit) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* node = (ActionNode*)curr_node;
		curr_node = node->next_node;
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* node = (ScopeNode*)curr_node;
		node->inner_random_exit_activate(curr_node,
										 scope_context,
										 node_context,
										 exit_depth,
										 exit_node,
										 random_curr_depth,
										 random_throw_id,
										 random_exceeded_limit);
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;
		node->inner_random_exit_activate(curr_node,
										 scope_context,
										 node_context);
	} else {
		ExitNode* node = (ExitNode*)curr_node;
		if (node->throw_id != -1) {
			random_throw_id = node->throw_id;
		} else {
			exit_depth = node->exit_depth-1;
			exit_node = node->next_node;
		}
	}
}

void Scope::inner_random_exit_activate(vector<Scope*>& scope_context,
									   vector<AbstractNode*>& node_context,
									   int& exit_depth,
									   AbstractNode*& exit_node,
									   int& random_curr_depth,
									   int& random_throw_id,
									   bool& random_exceeded_limit) {
	if (random_curr_depth > solution->depth_limit) {
		random_exceeded_limit = true;
		return;
	}
	random_curr_depth++;

	AbstractNode* curr_node = this->starting_node;
	while (true) {
		if (random_exceeded_limit
				|| random_throw_id != -1
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		node_inner_random_exit_activate_helper(curr_node,
											   scope_context,
											   node_context,
											   exit_depth,
											   exit_node,
											   random_curr_depth,
											   random_throw_id,
											   random_exceeded_limit);
	}

	random_curr_depth--;
}
