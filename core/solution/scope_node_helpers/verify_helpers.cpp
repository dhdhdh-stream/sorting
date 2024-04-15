#if defined(MDEBUG) && MDEBUG

#include "scope_node.h"

#include <iostream>

#include "scope.h"

using namespace std;

void ScopeNode::verify_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								int& exit_depth,
								AbstractNode*& exit_node,
								RunHelper& run_helper,
								ScopeNodeHistory* history) {
	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	history->scope_history = scope_history;
	context.back().scope_history = scope_history;

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->scope->verify_activate(problem,
								 context,
								 inner_exit_depth,
								 inner_exit_node,
								 run_helper,
								 scope_history);

	context.pop_back();

	context.back().node = NULL;

	if (run_helper.exceeded_limit) {
		// do nothing
	} else if (inner_exit_depth == -1) {
		curr_node = this->next_node;
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}

#endif /* MDEBUG */