#include "obs_node.h"

#include <iostream>

#include "factor.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void ObsNode::step(vector<double>& obs,
				   string& action,
				   bool& is_next,
				   SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ObsNodeHistory* history = new ObsNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	history->obs_history = obs;

	history->factor_initialized = vector<bool>(this->factors.size(), false);
	history->factor_values = vector<double>(this->factors.size());

	wrapper->node_context.back() = this->next_node;
}
