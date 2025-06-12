#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ActionNode::experiment_activate(AbstractNode*& curr_node,
									 Problem* problem,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	problem->perform_action(this->action);

	run_helper.num_actions++;

	curr_node = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->activate(
			curr_node,
			problem,
			run_helper,
			scope_history);
	}
}
