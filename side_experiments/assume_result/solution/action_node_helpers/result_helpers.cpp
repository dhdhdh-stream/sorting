#include "action_node.h"

#include <iostream>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "solution.h"

using namespace std;

void ActionNode::result_activate(AbstractNode*& curr_node,
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
	Minesweeper* minesweeper = (Minesweeper*)problem;
	context.back().location_history[this] = {minesweeper->current_x, minesweeper->current_y};
}
