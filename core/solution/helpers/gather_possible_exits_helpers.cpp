#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
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
								  bool& has_exited,
								  int& exit_depth,
								  AbstractNode*& exit_node) {
	bool inner_has_exited = false;
	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;
	if (l_index < (int)scope_context.size()-1) {
		gather_possible_exits_helper(l_index+1,
									 possible_exits,
									 context,
									 scope_context,
									 node_context,
									 inner_has_exited,
									 inner_exit_depth,
									 inner_exit_node);
	}

	if (inner_has_exited) {
		has_exited = true;
	} else {
		if (inner_exit_depth == -1) {
			Scope* scope = context[context.size() - scope_context.size() + l_index].scope;
			
			AbstractNode* starting_node;
			AbstractNode* experiment_node = scope->nodes[node_context[l_index]];
			if (experiment_node->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)experiment_node;
				starting_node = action_node->next_node;
			} else if (experiment_node->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)experiment_node;
				starting_node = scope_node->next_node;
			} else {
				/**
				 * - CleanExperiment on BranchNode edge case
				 */
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
										has_exited,
										exit_depth,
										exit_node,
										scope_context.size()-1 - l_index,
										possible_exits);
		} else if (inner_exit_depth == 0) {
			Scope* scope = context[context.size() - scope_context.size() + l_index].scope;
			scope->random_exit_activate(inner_exit_node,
										scope_context,
										node_context,
										has_exited,
										exit_depth,
										exit_node,
										scope_context.size()-1 - l_index,
										possible_exits);
		} else {
			exit_depth = inner_exit_depth-1;
			exit_node = inner_exit_node;
		}
	}
}

void gather_possible_exits(vector<pair<int,AbstractNode*>>& possible_exits,
						   vector<ContextLayer>& context,
						   vector<int>& scope_context,
						   vector<int>& node_context) {
	// unused
	bool has_exited = false;
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	gather_possible_exits_helper(0,
								 possible_exits,
								 context,
								 scope_context,
								 node_context,
								 has_exited,
								 exit_depth,
								 exit_node);
}

void parent_pass_through_gather_possible_exits_helper(
		int l_index,
		vector<pair<int,AbstractNode*>>& possible_exits,
		vector<ContextLayer>& context,
		vector<int>& scope_context,
		vector<int>& node_context,
		int parent_exit_depth,
		AbstractNode* parent_exit_node,
		bool& has_exited,
		int& exit_depth,
		AbstractNode*& exit_node) {
	bool inner_has_exited = false;
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
			inner_has_exited,
			inner_exit_depth,
			inner_exit_node);
	}

	if (inner_has_exited) {
		has_exited = true;
	} else {
		if (inner_exit_depth == -1) {
			Scope* scope = context[context.size() - scope_context.size() + l_index].scope;

			AbstractNode* starting_node;
			if (l_index == (int)scope_context.size()-1 - parent_exit_depth) {
				starting_node = parent_exit_node;
			} else {
				AbstractNode* experiment_node = scope->nodes[node_context[l_index]];
				if (experiment_node->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)experiment_node;
					starting_node = action_node->next_node;
				} else {
					ScopeNode* scope_node = (ScopeNode*)experiment_node;
					starting_node = scope_node->next_node;
				}
			}

			scope->random_exit_activate(starting_node,
										scope_context,
										node_context,
										has_exited,
										exit_depth,
										exit_node,
										scope_context.size()-1 - l_index,
										possible_exits);
		} else if (inner_exit_depth == 0) {
			Scope* scope = context[context.size() - scope_context.size() + l_index].scope;
			scope->random_exit_activate(inner_exit_node,
										scope_context,
										node_context,
										has_exited,
										exit_depth,
										exit_node,
										scope_context.size()-1 - l_index,
										possible_exits);
		} else {
			exit_depth = inner_exit_depth-1;
			exit_node = inner_exit_node;
		}
	}
}

void parent_pass_through_gather_possible_exits(vector<pair<int,AbstractNode*>>& possible_exits,
											   vector<ContextLayer>& context,
											   vector<int>& scope_context,
											   vector<int>& node_context,
											   int parent_exit_depth,
											   AbstractNode* parent_exit_node) {
	// unused
	bool has_exited = false;
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	parent_pass_through_gather_possible_exits_helper(0,
													 possible_exits,
													 context,
													 scope_context,
													 node_context,
													 parent_exit_depth,
													 parent_exit_node,
													 has_exited,
													 exit_depth,
													 exit_node);
}
