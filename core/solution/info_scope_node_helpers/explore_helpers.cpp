#include "info_scope_node.h"

#include "info_scope.h"
#include "scope.h"

using namespace std;

void InfoScopeNode::explore_activate(Problem* problem,
									 RunHelper& run_helper) {
	ScopeHistory* inner_scope_history;
	bool inner_is_positive;
	this->scope->activate(problem,
						  run_helper,
						  inner_scope_history,
						  inner_is_positive);
	delete inner_scope_history;
}
