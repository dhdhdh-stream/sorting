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

	this->scope->activate(problem,
						  context,
						  run_helper,
						  scope_history);

	delete scope_history;

	context.pop_back();

	context.back().node = NULL;
}
