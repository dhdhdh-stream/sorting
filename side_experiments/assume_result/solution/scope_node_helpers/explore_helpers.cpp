#include "scope_node.h"

#include <iostream>

#include "scope.h"

using namespace std;

void ScopeNode::explore_activate(Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	this->scope->activate(problem,
						  context,
						  run_helper);

	run_helper.num_actions++;
}
