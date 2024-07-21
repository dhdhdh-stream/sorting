#include "scope_node.h"

#include <iostream>

#include "scope.h"

using namespace std;

void ScopeNode::explore_activate(Problem* problem,
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

		this->scope->activate(problem,
							  context,
							  run_helper);

		context.back().node = NULL;
	}

	run_helper.num_actions++;
}
