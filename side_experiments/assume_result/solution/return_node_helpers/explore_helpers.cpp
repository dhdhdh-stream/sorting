#include "return_node.h"

#include "globals.h"
#include "problem.h"

using namespace std;

void ReturnNode::explore_activate(Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper) {
	if (this->previous_location != NULL) {
		map<AbstractNode*, vector<double>>::iterator it
			= context.back().location_history.find(this->previous_location);
		if (it != context.back().location_history.end()) {
			vector<double> world_location = problem_type->relative_to_world(
				it->second, this->location);
			problem->return_to_location(world_location,
										run_helper.num_analyze,
										run_helper.num_actions);
		}
	}

	run_helper.num_actions++;
}
