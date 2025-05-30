#include "scope_node.h"

#include "scope.h"

using namespace std;

void ScopeNode::commit_activate(Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;
	this->scope->experiment_activate(problem,
									 run_helper,
									 inner_scope_history);
}
