#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"
#include "solution_set.h"

using namespace std;

void ScopeNode::result_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper) {
	context.back().node = this;

	this->scope->result_activate(problem,
								 context,
								 run_helper);

	context.back().node = NULL;

	curr_node = this->next_node;

	run_helper.num_actions++;
	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	Minesweeper* minesweeper = (Minesweeper*)problem;
	context.back().location_history[this] = {minesweeper->current_x, minesweeper->current_y};
}
