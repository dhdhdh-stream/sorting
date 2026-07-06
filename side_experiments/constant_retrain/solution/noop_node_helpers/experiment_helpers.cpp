#include "noop_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void NoopNode::experiment_step(vector<double>& obs,
							   int& action,
							   bool& is_next,
							   SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	NoopNodeHistory* history = new NoopNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	if (!wrapper->should_explore) {
		history->obs = obs;
	}

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->experiment_check_activate(
			obs,
			wrapper);
	}
}
