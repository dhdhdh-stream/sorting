#include "action_node.h"

#include <iostream>

#include "problem.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper) {
	if (!run_helper.is_random()) {
		problem->perform_action(this->action);
	}

	curr_node = this->next_node;
}
