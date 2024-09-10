#include "absolute_return_node.h"

#include "globals.h"
#include "problem.h"

using namespace std;

void AbsoluteReturnNode::explore_activate(Problem* problem,
										  vector<ContextLayer>& context,
										  RunHelper& run_helper) {
	vector<double> world_location = problem_type->relative_to_world(
		context.back().starting_location, this->location);
	problem->return_to_location(world_location);

	run_helper.num_actions++;
}
