#ifndef ABSTRACT_DECISION_TREE_NODE_H
#define ABSTRACT_DECISION_TREE_NODE_H

#include <fstream>
#include <vector>

const int DECISION_TREE_NODE_TYPE_SPLIT = 0;
const int DECISION_TREE_NODE_TYPE_EVAL = 1;

class AbstractDecisionTreeNode {
public:
	int type;

	int id;

	virtual ~AbstractDecisionTreeNode() {};

	virtual void save(std::ofstream& output_file) = 0;
};

#endif /* ABSTRACT_DECISION_TREE_NODE_H */