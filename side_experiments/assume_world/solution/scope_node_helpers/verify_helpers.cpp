#if defined(MDEBUG) && MDEBUG

#include "scope_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"

using namespace std;

void ScopeNode::verify_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper) {
	if (run_helper.scope_node_ancestors.find(this) != run_helper.scope_node_ancestors.end()) {
		run_helper.exceeded_limit = true;
		return;
	}

	context.back().node = this;
	run_helper.scope_node_ancestors.insert(this);

	this->scope->verify_activate(problem,
								 context,
								 run_helper);

	context.back().node = NULL;
	run_helper.scope_node_ancestors.erase(this);

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
}

#endif /* MDEBUG */