#include "scope_node.h"

using namespace std;

void ScopeNode::potential_activate(Problem* problem,
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

	// unused
	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->scope->activate(problem,
						  context,
						  inner_exit_depth,
						  inner_exit_node,
						  run_helper,
						  scope_history);

	context.pop_back();

	context.back().node = NULL;
}
