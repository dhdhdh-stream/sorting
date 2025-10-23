#include "start_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void StartNode::experiment_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	StartNodeHistory* history = new StartNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	history->num_actions_snapshot = wrapper->num_actions;

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			false,
			wrapper);
	}
}
