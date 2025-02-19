#include "scope_node.h"

#include "scope.h"

using namespace std;

void ScopeNode::measure_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 RunHelper& run_helper,
								 ScopeHistory* scope_history) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;
	this->scope->activate(problem,
						  run_helper,
						  inner_scope_history);

	curr_node = this->next_node;
}
