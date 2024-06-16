#include "info_scope_node.h"

#include "info_scope.h"
#include "scope.h"

using namespace std;

void InfoScopeNode::explore_activate(Problem* problem,
									 RunHelper& run_helper) {
	double inner_score;
	this->scope->activate(problem,
						  run_helper,
						  inner_score);
}
