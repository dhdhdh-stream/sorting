#include "action_tree_node.h"

using namespace std;

void ActionTreeNode::activate(Problem* problem,
							  map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	ActionNodeHistory* history = new ActionNodeHistory();
	history->index = (int)node_histories.size();
	node_histories[this->action_node] = history;

	problem->perform_action(this->action_node->action);

	history->obs_snapshot = problem->get_observations();
}
