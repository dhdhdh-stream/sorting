#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "scope.h"

using namespace std;

void ScopeNode::explore_activate(Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	context.back().node = this;

	this->scope->activate(problem,
						  context,
						  run_helper);

	context.back().node = NULL;
}
