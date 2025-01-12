#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::new_scope_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	context.back().node_id = this->id;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;
	this->scope->new_scope_activate(problem,
									context,
									run_helper,
									inner_scope_history);

	curr_node = this->next_node;

	/**
	 * - still need to check experiments
	 */
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->activate(
			this,
			false,
			curr_node,
			problem,
			context,
			run_helper,
			scope_history);
	}
}

#if defined(MDEBUG) && MDEBUG
void ScopeNode::new_scope_capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	context.back().node_id = this->id;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	this->scope->new_scope_capture_verify_activate(problem,
												   context,
												   run_helper,
												   inner_scope_history);
	delete inner_scope_history;

	curr_node = this->next_node;

	/**
	 * - still need to check experiments
	 */
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->activate(
			this,
			false,
			curr_node,
			problem,
			context,
			run_helper,
			NULL);
	}
}
#endif /* MDEBUG */
