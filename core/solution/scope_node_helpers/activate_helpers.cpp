#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::activate(AbstractNode*& curr_node,
						 Problem* problem,
						 vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	context.back().node = this;

	this->scope->activate(problem,
						  context,
						  run_helper);

	context.back().node = NULL;

	ScopeNodeHistory* history = new ScopeNodeHistory();
	history->index = node_histories.size();
	node_histories[this] = history;
	history->obs_snapshot = problem->get_observations();

	curr_node = this->next_node;

	if (!run_helper.exceeded_limit) {
		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				this,
				false,
				curr_node,
				problem,
				context,
				run_helper);
			if (is_selected) {
				return;
			}
		}
	}

	if (run_helper.experiments_seen_order.size() == 0) {
		map<Scope*, set<pair<AbstractNode*,bool>>>::iterator scope_it = run_helper.scope_nodes_seen.find((Scope*)this->parent);
		if (scope_it == run_helper.scope_nodes_seen.end()) {
			scope_it = run_helper.scope_nodes_seen.insert({(Scope*)this->parent, set<pair<AbstractNode*,bool>>()}).first;
		}
		scope_it->second.insert({this, false});
	}
}
