#if defined(MDEBUG) && MDEBUG

#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_set.h"

using namespace std;

void ScopeNode::verify_activate(AbstractNode*& curr_node,
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

		this->scope->verify_activate(problem,
									 context,
									 run_helper);

		context.back().node = NULL;
	}

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

#endif /* MDEBUG */