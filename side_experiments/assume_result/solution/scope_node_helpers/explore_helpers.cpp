#include "scope_node.h"

#include <iostream>

#include "scope.h"

using namespace std;

void ScopeNode::explore_activate(Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	if (run_helper.scope_node_ancestors.find(this) != run_helper.scope_node_ancestors.end()) {
		run_helper.exceeded_limit = true;
		return;
	}

	context.back().node = this;
	run_helper.scope_node_ancestors.insert(this);

	this->scope->activate(problem,
						  context,
						  run_helper);

	context.back().node = NULL;
	run_helper.scope_node_ancestors.erase(this);
}
