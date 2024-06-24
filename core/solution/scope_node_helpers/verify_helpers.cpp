#if defined(MDEBUG) && MDEBUG

#include "scope_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"

using namespace std;

void ScopeNode::verify_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper,
								ScopeNodeHistory* history) {
	context.back().node = this;

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	this->scope->verify_activate(problem,
								 context,
								 run_helper,
								 scope_history);

	context.back().node = NULL;

	history->scope_history = scope_history;

	curr_node = this->next_node;
}

#endif /* MDEBUG */