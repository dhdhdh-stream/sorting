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
	ScopeNodeHistory* history = new ScopeNodeHistory();
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

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	if (run_helper.experiments_seen_order.size() == 0) {
		map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.nodes_seen.find({this, false});
		if (it == run_helper.nodes_seen.end()) {
			run_helper.nodes_seen[{this, false}] = 1;
		} else {
			it->second++;
		}
	}

	if (!run_helper.exceeded_limit) {
		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				this,
				false,
				curr_node,
				problem,
				context,
				run_helper,
				scope_history);
			if (is_selected) {
				return;
			}
		}
	}
}
