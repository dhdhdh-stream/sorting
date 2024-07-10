#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_random_exit_activate_helper(AbstractNode*& curr_node,
									  vector<AbstractNode*>& possible_exits) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;

			possible_exits.push_back(curr_node);

			curr_node = node->next_node;
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;

			possible_exits.push_back(curr_node);

			curr_node = node->next_node;
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;

			possible_exits.push_back(curr_node);

			uniform_int_distribution<int> distribution(0, 1);
			if (distribution(generator) == 0) {
				curr_node = node->branch_next_node;
			} else {
				curr_node = node->original_next_node;
			}
		}

		break;
	}
}

void Scope::random_exit_activate(AbstractNode* starting_node,
								 vector<AbstractNode*>& possible_exits) {
	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_random_exit_activate_helper(curr_node,
										 possible_exits);
	}
}
