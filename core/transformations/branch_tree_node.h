#ifndef BRANCH_TREE_NODE_H
#define BRANCH_TREE_NODE_H

#include "abstract_tree_node.h"

class BranchTreeNode : public AbstractTreeNode {
public:
	AbstractNode* branch_node;

	SequenceTreeNode* original_path;
	SequenceTreeNode* branch_path;

	BranchTreeNode();
	BranchTreeNode(BranchTreeNode* original);
	~BranchTreeNode();

	void activate(Problem* problem,
				  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);
};

#endif /* BRANCH_TREE_NODE_H */