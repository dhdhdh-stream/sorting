#include "sequence_tree_node.h"

using namespace std;

void SequenceTreeNode::activate(Problem* problem,
								map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		switch (this->nodes[n_index]->type) {
		case TREE_NODE_TYPE_ACTION:
			{
				ActionTreeNode* action_tree_node = (ActionTreeNode*)this->nodes[n_index];
				action_tree_node->activate(problem,
										   node_histories);
			}
			break;
		case TREE_NODE_TYPE_BRANCH:
			{
				BranchTreeNode* branch_tree_node = (BranchTreeNode*)this->nodes[n_index];
				branch_tree_node->activate(problem,
										   node_histories);
			}
			break;
		}
	}
}
