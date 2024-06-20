#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	ActionNodeHistory* history = new ActionNodeHistory();
	history->index = (int)node_histories.size();
	node_histories[this] = history;

	problem->perform_action(this->action);

	history->obs_snapshot = problem->get_observations();

	curr_node = this->next_node;

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

	if (run_helper.experiments_seen_order.size() == 0) {
		if (this->parent->type == SCOPE_TYPE_SCOPE) {
			map<Scope*, set<pair<AbstractNode*,bool>>>::iterator scope_it = run_helper.scope_nodes_seen.find((Scope*)this->parent);
			if (scope_it == run_helper.scope_nodes_seen.end()) {
				scope_it = run_helper.scope_nodes_seen.insert({(Scope*)this->parent, set<pair<AbstractNode*,bool>>()}).first;
			}
			scope_it->second.insert({this, false});
		} else {
			map<InfoScope*, set<AbstractNode*>>::iterator scope_it = run_helper.info_scope_nodes_seen.find((InfoScope*)this->parent);
			if (scope_it == run_helper.info_scope_nodes_seen.end()) {
				scope_it = run_helper.info_scope_nodes_seen.insert({(InfoScope*)this->parent, set<AbstractNode*>()}).first;
			}
			scope_it->second.insert({this});
		}
	}
}
