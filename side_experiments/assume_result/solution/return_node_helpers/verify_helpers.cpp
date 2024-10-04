#if defined(MDEBUG) && MDEBUG

#include "return_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "new_action_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ReturnNode::verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	bool is_branch = false;
	if (this->previous_location != NULL) {
		map<AbstractNode*, vector<double>>::iterator it
			= context.back().location_history.find(this->previous_location);
		if (it != context.back().location_history.end()) {
			vector<double> world_location = problem_type->relative_to_world(
				it->second, this->location);
			problem->return_to_location(world_location);

			is_branch = true;
		}
	}

	if (is_branch) {
		curr_node = this->passed_next_node;
	} else {
		curr_node = this->skipped_next_node;
	}

	run_helper.num_actions++;
	if (run_helper.num_actions > solution_duplicate->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}

	if (solution_duplicate->subproblem_starting_node == this) {
		solution_duplicate->subproblem->verify_activate(problem,
														context,
														run_helper);

		curr_node = solution_duplicate->subproblem_exit_node;
	}
}

#endif /* MDEBUG */