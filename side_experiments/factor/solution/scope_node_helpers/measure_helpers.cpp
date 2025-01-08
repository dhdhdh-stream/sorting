#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::measure_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	context.back().node_id = this->id;

	this->scope->measure_activate(problem,
								  context,
								  run_helper);

	curr_node = this->next_node;
}
