#include "absolute_return_node.h"

#include "problem.h"

using namespace std;

void AbsoluteReturnNode::explore_activate(Problem* problem,
										  vector<ContextLayer>& context,
										  RunHelper& run_helper) {
	problem->return_to_location(context.back().starting_location,
								this->location);

	run_helper.num_actions++;
}
