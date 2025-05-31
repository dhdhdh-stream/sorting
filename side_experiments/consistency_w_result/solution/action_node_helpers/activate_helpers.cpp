#include "action_node.h"

#include <iostream>

#include "problem.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper) {
	problem->perform_action(this->action);

	run_helper.num_true_actions++;

	curr_node = this->next_node;
}
