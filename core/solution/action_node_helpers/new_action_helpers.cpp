#include "action_node.h"

#include "problem.h"

using namespace std;

void ActionNode::new_action_activate(AbstractNode*& curr_node,
									 Problem* problem,
									 map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	ActionNodeHistory* history = new ActionNodeHistory();
	history->index = (int)node_histories.size();
	node_histories[this] = history;

	problem->perform_action(this->action);
	history->obs_snapshot = problem->get_observations();

	curr_node = this->next_node;
}
