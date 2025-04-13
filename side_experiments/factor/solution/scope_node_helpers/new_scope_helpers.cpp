#if defined(MDEBUG) && MDEBUG

#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::new_scope_capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;
	this->scope->new_scope_capture_verify_activate(problem,
												   run_helper,
												   inner_scope_history);

	curr_node = this->next_node;

	/**
	 * - still need to check experiments
	 */
	if (this->experiment != NULL) {
		this->experiment->activate(
			this,
			false,
			curr_node,
			problem,
			run_helper,
			scope_history);
	}
}

#endif /* MDEBUG */