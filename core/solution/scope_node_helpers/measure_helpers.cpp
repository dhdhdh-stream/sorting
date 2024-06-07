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
	context.back().node = this;

	this->scope->measure_activate(metrics,
								  problem,
								  context,
								  run_helper);

	context.back().node = NULL;

	ScopeNodeHistory* history = new ScopeNodeHistory();
	history->index = node_histories.size();
	node_histories[this] = history;
	history->obs_snapshot = problem->get_observations();

	curr_node = this->next_node;
}
