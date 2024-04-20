#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "scope.h"

using namespace std;

void ScopeNode::explore_activate(Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	context.back().scope_history = scope_history;

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
