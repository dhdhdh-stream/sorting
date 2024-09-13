#include "return_node.h"

#include "problem.h"

using namespace std;

void ReturnNode::explore_activate(Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper) {
	if (this->previous_location != NULL) {
		map<AbstractNode*, pair<vector<int>,vector<double>>>::iterator it
			= context.back().node_history.find(this->previous_location);
		if (it != context.back().node_history.end()) {
			problem->return_to_location(it->second.first);
		}
	}

	run_helper.num_actions++;
}
