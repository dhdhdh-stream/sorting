#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void ActionNode::experiment_activate(AbstractNode*& curr_node,
									 Problem* problem,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	while (run_helper.is_random()) {
		#if defined(MDEBUG) && MDEBUG
		problem->perform_action(Action(run_helper.curr_run_seed%problem_type->num_possible_actions()));
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
		problem->perform_action(Action(action_distribution(generator)));
		#endif /* MDEBUG */
	}

	if (!run_helper.is_random()) {
		problem->perform_action(this->action);
	}

	curr_node = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->activate(
			this,
			false,
			curr_node,
			problem,
			run_helper,
			scope_history);
	}
}
