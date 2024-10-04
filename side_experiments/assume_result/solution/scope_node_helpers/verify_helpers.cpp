#if defined(MDEBUG) && MDEBUG

#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::verify_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper) {
	this->scope->verify_activate(problem,
								 context,
								 run_helper);

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution_duplicate->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().location_history[this] = problem->get_location();

	if (!run_helper.exceeded_limit) {
		if (solution_duplicate->subproblem_starting_node == this) {
			solution_duplicate->subproblem->verify_activate(problem,
															context,
															run_helper);

			curr_node = solution_duplicate->subproblem_exit_node;
		}
	}
}

#endif /* MDEBUG */