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
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	problem->perform_action(this->action);
	history->obs_snapshot = problem->get_observation();

	curr_node = this->next_node;

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->activate(
			curr_node,
			problem,
			context,
			exit_depth,
			run_helper);
		if (is_selected) {
			return;
		}
	}
}
