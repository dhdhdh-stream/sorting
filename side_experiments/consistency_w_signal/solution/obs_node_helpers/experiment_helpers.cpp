#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void ObsNode::experiment_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ObsNodeHistory* history = new ObsNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			false,
			wrapper);
	}
}
