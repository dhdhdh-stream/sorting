#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::measure_activate(Metrics& metrics,
								 AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	if (run_helper.scope_node_ancestors.find(this) != run_helper.scope_node_ancestors.end()) {
		run_helper.exceeded_limit = true;
		return;
	}

	context.back().node = this;
	run_helper.scope_node_ancestors.insert(this);

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	this->scope->measure_activate(metrics,
								  problem,
								  context,
								  run_helper,
								  scope_history);

	context.back().node = NULL;
	run_helper.scope_node_ancestors.erase(this);

	ScopeNodeHistory* history = new ScopeNodeHistory();
	history->index = node_histories.size();
	node_histories[this] = history;
	history->scope_history = scope_history;

	curr_node = this->next_node;
}
