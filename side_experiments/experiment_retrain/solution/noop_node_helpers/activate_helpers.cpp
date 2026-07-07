#include "noop_node.h"

#include <iostream>

#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void NoopNode::step(vector<double>& obs,
					int& action,
					bool& is_next,
					SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	NoopNodeHistory* history = new NoopNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	wrapper->node_context.back() = this->next_node;
}
