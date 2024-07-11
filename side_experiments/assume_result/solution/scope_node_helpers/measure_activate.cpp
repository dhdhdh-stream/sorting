#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "solution_set.h"

using namespace std;

void ScopeNode::measure_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 vector<int>& scope_counts) {
	if (run_helper.scope_node_ancestors.find(this) != run_helper.scope_node_ancestors.end()) {
		run_helper.exceeded_limit = true;
		return;
	}

	context.back().node = this;
	run_helper.scope_node_ancestors.insert(this);

	this->scope->measure_activate(problem,
								  context,
								  run_helper,
								  scope_counts);

	context.back().node = NULL;
	run_helper.scope_node_ancestors.erase(this);

	curr_node = this->next_node;

	run_helper.num_actions++;
	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().nodes_seen.push_back({this, false});
}
