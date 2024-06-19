#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void InfoScope::random_exit_activate(AbstractNode* starting_node,
									 vector<AbstractNode*>& possible_exits) {
	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		ActionNode* node = (ActionNode*)curr_node;

		possible_exits.push_back(curr_node);

		curr_node = node->next_node;
	}
}
