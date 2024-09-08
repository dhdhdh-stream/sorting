#include "absolute_return_node.h"

#include "globals.h"
#include "problem.h"
#include "solution.h"

using namespace std;

void AbsoluteReturnNode::result_activate(AbstractNode*& curr_node,
										 Problem* problem,
										 vector<ContextLayer>& context,
										 RunHelper& run_helper) {
	problem->return_to_location(context.back().starting_location,
								this->location);

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
}
