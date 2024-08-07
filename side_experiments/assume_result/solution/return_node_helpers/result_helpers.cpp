#include "return_node.h"

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "solution.h"

using namespace std;

void ReturnNode::result_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	if (this->previous_location != NULL) {
		map<AbstractNode*, pair<int,int>>::iterator it
			= context.back().location_history.find(this->previous_location);
		if (it != context.back().location_history.end()) {
			Minesweeper* minesweeper = (Minesweeper*)problem;
			minesweeper->current_x = it->second.first;
			minesweeper->current_y = it->second.second;
		}
	}

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
}
