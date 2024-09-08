#include "return_node.h"

#include "globals.h"
#include "problem.h"
#include "solution.h"

using namespace std;

void ReturnNode::result_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	bool is_branch = false;
	if (this->previous_location != NULL) {
		map<AbstractNode*, ProblemLocation*>::iterator it
			= context.back().location_history.find(this->previous_location);
		if (it != context.back().location_history.end()) {
			problem->return_to_location(context.back().starting_location,
										it->second);

			is_branch = true;
		}
	}

	if (is_branch) {
		curr_node = this->passed_next_node;
	} else {
		curr_node = this->skipped_next_node;
	}

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
}
