#include "action_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ActionNode::explore_activate(Problem* problem,
								  RunHelper& run_helper) {
	problem->perform_action(this->action);

	run_helper.num_actions++;
}
