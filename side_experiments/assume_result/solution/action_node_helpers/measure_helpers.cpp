#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "new_action_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ActionNode::measure_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper) {
	problem->perform_action(this->action);

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution_duplicate->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().location_history[this] = problem->get_location();

	if (solution_duplicate->subproblem_starting_node == this) {
		solution_duplicate->subproblem->measure_activate(problem,
														 context,
														 run_helper);

		curr_node = solution_duplicate->subproblem_exit_node;
	}
}
