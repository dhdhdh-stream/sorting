#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "factor.h"
#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void ObsNode::experiment_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ObsNodeHistory* history = new ObsNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	history->obs_history = obs;

	for (int f_index = 0; f_index < (int)this->impacted_factors.size(); f_index++) {
		wrapper->scope_histories.back()->factor_initialized[
			this->impacted_factors[f_index]] = false;
	}

	history->num_actions_snapshot = wrapper->num_actions;

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			false,
			wrapper);
	}
}
