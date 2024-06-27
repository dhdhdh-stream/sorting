#ifndef SEQUENCE_TREE_NODE_H
#define SEQUENCE_TREE_NODE_H

#include "abstract_tree_node.h"

class SequenceTreeNode : public AbstractTreeNode {
public:
	std::vector<AbstractTreeNode*> nodes;

	SequenceTreeNode();
	SequenceTreeNode(SequenceTreeNode* original);
	~SequenceTreeNode();

	void activate(Problem* problem,
				  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);
};

#endif /* SEQUENCE_TREE_NODE_H */