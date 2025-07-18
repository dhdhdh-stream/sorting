#include "action_node.h"

#include <iostream>

#include "solution_wrapper.h"

using namespace std;

void ActionNode::step(vector<double>& obs,
					  string& action,
					  bool& is_next,
					  SolutionWrapper* wrapper) {
	action = this->action;
	is_next = true;

	wrapper->num_actions++;

	wrapper->node_context.back() = this->next_node;
}
