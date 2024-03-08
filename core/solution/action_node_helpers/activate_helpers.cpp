#include "action_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	problem->perform_action(this->action);
	history->obs_snapshot = problem->get_observation();

	curr_node = this->next_node;

	if (run_helper.can_restart) {
		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%((int)solution->average_num_actions + 10) == 0) {
			run_helper.should_restart = true;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		uniform_int_distribution<int> restart_distribution(0, (int)solution->average_num_actions + 10);
		if (restart_distribution(generator) == 0) {
			run_helper.should_restart = true;
		}
		#endif /* MDEBUG */
	}

	if (!run_helper.should_restart) {
		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper);
			if (is_selected) {
				return;
			}
		}
	}
}
