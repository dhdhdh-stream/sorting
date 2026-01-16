#include "obs_node.h"

#include <iostream>

#include "scope.h"
#include "solution_wrapper.h"
#include "problem.h"

using namespace std;

void ObsNode::step(vector<double>& obs,
				   int& action,
				   bool& is_next,
				   SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ObsNodeHistory* history = new ObsNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	history->obs_history = obs;

	wrapper->node_context.back() = this->next_node;
}
