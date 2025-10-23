#include "action_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::step(vector<double>& obs,
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

	for (int f_index = 0; f_index < (int)this->impacted_factors.size(); f_index++) {
		wrapper->scope_histories.back()->factor_initialized[
			this->impacted_factors[f_index]] = false;
	}

	history->num_actions_snapshot = wrapper->num_actions;

	wrapper->node_context.back() = this->next_node;
}
