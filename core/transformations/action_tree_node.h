#ifndef ACTION_TREE_NODE_H
#define ACTION_TREE_NODE_H

#include "abstract_tree_node.h"

class ActionTreeNode : public AbstractTreeNode {
public:
	ActionNode* action_node;

	ActionNode();
	ActionNode(ActionNode* original);
	~ActionNode();

	void activate(Problem* problem,
				  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);
};

#endif /* ACTION_TREE_NODE_H */