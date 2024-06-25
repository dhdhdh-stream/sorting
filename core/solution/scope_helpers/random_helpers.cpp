#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_random_activate_helper(AbstractNode*& curr_node,
								 vector<AbstractNode*>& possible_nodes) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;

			possible_nodes.push_back(curr_node);

			curr_node = node->next_node;
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;

			possible_nodes.push_back(curr_node);

			curr_node = node->next_node;
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;

			possible_nodes.push_back(curr_node);

			uniform_int_distribution<int> distribution(0, 1);
			if (distribution(generator) == 0) {
				curr_node = node->branch_next_node;
			} else {
				curr_node = node->original_next_node;
			}
		}

		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* node = (InfoBranchNode*)curr_node;

			possible_nodes.push_back(curr_node);

			uniform_int_distribution<int> distribution(0, 1);
			if (distribution(generator) == 0) {
				curr_node = node->branch_next_node;
			} else {
				curr_node = node->original_next_node;
			}
		}

		break;
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* node = (BranchEndNode*)curr_node;

			possible_nodes.push_back(curr_node);

			curr_node = node->next_node;
		}

		break;
	}
}

void Scope::random_activate(AbstractNode* starting_node,
							vector<AbstractNode*>& possible_nodes) {
	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_random_activate_helper(curr_node,
									possible_nodes);
	}
}

void Scope::random_exit_activate(AbstractNode* node_context,
								 bool is_branch,
								 vector<AbstractNode*>& possible_pre_exits,
								 vector<AbstractNode*>& possible_exits) {
	AbstractNode* curr_pre_node = node_context;
	AbstractNode* curr_node;
	switch (node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node_context;
			curr_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)node_context;
			curr_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node_context;
			if (is_branch) {
				curr_node = branch_node->branch_next_node;
			} else {
				curr_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)node_context;
			if (is_branch) {
				curr_node = info_branch_node->branch_next_node;
			} else {
				curr_node = info_branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* branch_end_node = (BranchEndNode*)node_context;
			curr_node = branch_end_node->next_node;
		}
		break;
	}

	/**
	 * - end node edge case
	 */
	if (curr_node == NULL) {
		possible_pre_exits.push_back(curr_pre_node);
		possible_exits.push_back(curr_node);

		return;
	}

	while (true) {
		possible_pre_exits.push_back(curr_pre_node);
		possible_exits.push_back(curr_node);

		switch (curr_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;
				curr_pre_node = node;
				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)curr_node;
				curr_pre_node = node;
				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;
				curr_pre_node = node->branch_end_node;
				curr_node = node->branch_end_node->next_node;
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* node = (InfoBranchNode*)curr_node;
				curr_pre_node = node->branch_end_node;
				curr_node = node->branch_end_node->next_node;
			}
			break;
		// if NODE_TYPE_BRANCH_END, do nothing
		}

		if (curr_node == NULL
				|| curr_node->type == NODE_TYPE_BRANCH_END) {
			break;
		}
	}
}
