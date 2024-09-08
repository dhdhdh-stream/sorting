#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::measure_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	bool previously_seen = false;
	for (int l_index = (int)context.size()-2; l_index >= 0; l_index--) {
		ScopeNode* previous_scope_node = (ScopeNode*)context[l_index].node;
		if (previous_scope_node == this) {
			previously_seen = true;
			break;
		} else if (context[l_index].scope == this->scope
				&& previous_scope_node->index > this->index) {
			break;
		}
	}

	if (!previously_seen) {
		context.back().node = this;

		this->scope->measure_activate(problem,
									  context,
									  run_helper);

		context.back().node = NULL;
	}

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().location_history[this] = problem->get_relative_location(context.back().starting_location);
}
