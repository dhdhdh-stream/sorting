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
	scope_history->node_histories.push_back(history);

	wrapper->node_context.back() = this->next_node;

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->experiment_check_activate(
			obs,
			wrapper);
	}
}
