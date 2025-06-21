#include "action_node.h"

#include <iostream>

#include "explore.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::explore_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	action = this->action;
	is_next = true;

	wrapper->num_actions++;

	wrapper->node_context.back() = this->next_node;

	if (this->explore != NULL) {
		this->explore->check_activate(wrapper);
	}
}
