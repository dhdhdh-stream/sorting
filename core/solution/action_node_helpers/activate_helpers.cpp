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
			map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.scope_nodes_seen.find({this, false});
			if (it == run_helper.scope_nodes_seen.end()) {
				run_helper.scope_nodes_seen[{this, false}] = 1;
			} else {
				it->second++;
			}
		} else {
			map<AbstractNode*, int>::iterator it = run_helper.info_scope_nodes_seen.find(this);
			if (it == run_helper.info_scope_nodes_seen.end()) {
				run_helper.info_scope_nodes_seen[this] = 1;
			} else {
				it->second++;
			}
		}
	}
}
