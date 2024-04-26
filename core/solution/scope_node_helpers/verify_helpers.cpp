#if defined(MDEBUG) && MDEBUG

#include "scope_node.h"

#include <iostream>

#include "scope.h"

using namespace std;

void ScopeNode::verify_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper,
								ScopeNodeHistory* history) {
	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	history->scope_history = scope_history;
	context.back().scope_history = scope_history;

	this->scope->verify_activate(problem,
								 context,
								 run_helper,
								 scope_history);

	context.pop_back();

	context.back().node = NULL;

	curr_node = this->next_node;
}

#endif /* MDEBUG */