#include "action_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "solution.h"
#include "solution_set.h"

using namespace std;

void ActionNode::measure_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper) {
	problem->perform_action(this->action);

	curr_node = this->next_node;

	run_helper.num_actions++;
	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().nodes_seen.push_back({this, false});
}
