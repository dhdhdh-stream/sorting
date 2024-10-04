#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::result_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper) {
	this->scope->result_activate(problem,
								 context,
								 run_helper);

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().location_history[this] = problem->get_location();

	if (!run_helper.exceeded_limit) {
		if (solution->subproblem_starting_node == this) {
			run_helper.hit_subproblem = true;

			solution->subproblem->result_activate(problem,
												  context,
												  run_helper);

			curr_node = solution->subproblem_exit_node;
		}
	}
}
