#ifndef ABSTRACT_TREE_NODE_H
#define ABSTRACT_TREE_NODE_H

const int TREE_NODE_TYPE_SEQUENCE = 0;
const int TREE_NODE_TYPE_REARRANGE = 1;

const int TREE_NODE_TYPE_ACTION = 2;
const int TREE_NODE_TYPE_SCOPE = 3;
const int TREE_NODE_TYPE_BRANCH = 4;

class AbstractTreeNode {
public:
	int type;

};

#endif /* ABSTRACT_TREE_NODE_H */