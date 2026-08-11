#include "noop_node.h"

#include <iostream>

#include "init_network.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void NoopNode::step(vector<double>& obs,
					int& action,
					bool& is_next,
					SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	NoopNodeHistory* history = new NoopNodeHistory(this);
	scope_history->node_histories.push_back(history);

	wrapper->node_context.back() = this->next_node;
}
