#include "action_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"

using namespace std;

void ActionNode::measure_match_activate(AbstractNode*& curr_node,
										Problem* problem,
										RunHelper& run_helper,
										ScopeHistory* scope_history) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	history->num_matches = run_helper.num_matches;
	scope_history->node_histories[this->id] = history;

	problem->perform_action(this->action);

	run_helper.num_true_actions++;

	curr_node = this->next_node;
}
