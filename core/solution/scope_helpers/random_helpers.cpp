#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "exit_node.h"
#include "globals.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_random_existing_activate_helper(AbstractNode*& curr_node,
										  vector<Scope*>& scope_context,
										  vector<AbstractNode*>& node_context,
										  int& exit_depth,
										  AbstractNode*& exit_node,
										  int& random_curr_depth,
										  int& random_throw_id,
										  bool& random_exceeded_limit,
										  vector<AbstractNode*>& possible_nodes) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;

			possible_nodes.push_back(node);

			curr_node = node->next_node;
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->random_existing_activate(curr_node,
										   scope_context,
										   node_context,
										   exit_depth,
										   exit_node,
										   random_curr_depth,
										   random_throw_id,
										   random_exceeded_limit,
										   possible_nodes);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->random_existing_activate(curr_node,
										   scope_context,
										   node_context,
										   possible_nodes);
		}

		break;
	case NODE_TYPE_EXIT:
		{
			ExitNode* node = (ExitNode*)curr_node;

			possible_nodes.push_back(node);

			if (node->throw_id != -1) {
				random_throw_id = node->throw_id;
			} else {
				exit_depth = node->exit_depth-1;
				exit_node = node->next_node;
			}
		}

		break;
	}
}

void Scope::random_existing_activate(AbstractNode* starting_node,
									 vector<Scope*>& scope_context,
									 vector<AbstractNode*>& node_context,
									 int& exit_depth,
									 AbstractNode*& exit_node,
									 int& random_curr_depth,
									 int& random_throw_id,
									 bool& random_exceeded_limit,
									 vector<AbstractNode*>& possible_nodes) {
	if (random_curr_depth > solution->depth_limit) {
		random_exceeded_limit = true;
		return;
	}
	random_curr_depth++;

	AbstractNode* curr_node = starting_node;
	while (true) {
		if (random_exceeded_limit
				|| random_throw_id != -1
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		if (scope_context.size() > 1) {
			ScopeNode* scope_node = (ScopeNode*)node_context[scope_context.size()-2];
			if (scope_node->exit_nodes.find(curr_node) != scope_node->exit_nodes.end()) {
				break;
			}
		}

		node_random_existing_activate_helper(curr_node,
											 scope_context,
											 node_context,
											 exit_depth,
											 exit_node,
											 random_curr_depth,
											 random_throw_id,
											 random_exceeded_limit,
											 possible_nodes);
	}

	random_curr_depth--;
}

void node_inner_random_existing_activate_helper(AbstractNode*& curr_node,
												vector<Scope*>& scope_context,
												vector<AbstractNode*>& node_context,
												int& exit_depth,
												AbstractNode*& exit_node,
												int& random_curr_depth,
												int& random_throw_id,
												bool& random_exceeded_limit) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			curr_node = node->next_node;
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->inner_random_existing_activate(curr_node,
												 scope_context,
												 node_context,
												 exit_depth,
												 exit_node,
												 random_curr_depth,
												 random_throw_id,
												 random_exceeded_limit);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->inner_random_existing_activate(curr_node,
												 scope_context,
												 node_context);
		}

		break;
	case NODE_TYPE_EXIT:
		{
			ExitNode* node = (ExitNode*)curr_node;
			if (node->throw_id != -1) {
				random_throw_id = node->throw_id;
			} else {
				exit_depth = node->exit_depth-1;
				exit_node = node->next_node;
			}
		}

		break;
	}
}

void Scope::inner_random_existing_activate(AbstractNode* starting_node,
										   vector<Scope*>& scope_context,
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

	AbstractNode* curr_node = starting_node;
	while (true) {
		if (random_exceeded_limit
				|| random_throw_id != -1
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		if (scope_context.size() > 1) {
			ScopeNode* scope_node = (ScopeNode*)node_context[scope_context.size()-2];
			if (scope_node->exit_nodes.find(curr_node) != scope_node->exit_nodes.end()) {
				break;
			}
		}

		node_inner_random_existing_activate_helper(curr_node,
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
