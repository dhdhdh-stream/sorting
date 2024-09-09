#include "return_node.h"

#include "problem.h"

using namespace std;

void ReturnNode::explore_activate(Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper) {
	if (this->previous_location != NULL) {
		map<AbstractNode*, vector<double>>::iterator it
			= context.back().location_history.find(this->previous_location);
		if (it != context.back().location_history.end()) {
			problem->return_to_location(it->second);
		}
	}

	run_helper.num_actions++;
}
