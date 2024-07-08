#include "action_node.h"

#include <iostream>

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper) {
	problem->perform_action(this->action);

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().nodes_seen.push_back({this, false});

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->activate(
			this,
			false,
			curr_node,
			problem,
			context,
			run_helper);
		if (is_selected) {
			return;
		}
	}
}
