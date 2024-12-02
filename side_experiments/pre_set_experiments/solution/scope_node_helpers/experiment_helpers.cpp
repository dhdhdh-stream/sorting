#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::experiment_activate(AbstractNode*& curr_node,
									Problem* problem,
									vector<ContextLayer>& context,
									RunHelper& run_helper,
									ScopeHistory* scope_history) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	context.back().node_id = this->id;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;
	this->scope->experiment_activate(problem,
									 context,
									 run_helper,
									 inner_scope_history);

	curr_node = this->next_node;

	if (run_helper.experiment_node == this) {
		this->experiment->activate(
			curr_node,
			problem,
			context,
			run_helper,
			scope_history);
	}
}
