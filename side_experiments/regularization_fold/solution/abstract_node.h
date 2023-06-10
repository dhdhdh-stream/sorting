#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_INNER_SCOPE = 1;
const int NODE_TYPE_FETCH = 2;
const int NODE_TYPE_BRANCH = 3;
const int NODE_TYPE_BRANCH_END = 4;	// TODO: can be from inner

// TODO: remove fold nodes
// - track experiments in action nodes and scope nodes
//   - assign target node if run passed without any experiment being hit

// TODO: add explore after action, scope, fetch, and branch_end?

// TODO: split score impact among scope node, fetch node
// - but scope node, fetch node still have their own score networks

class AbstractNode {
public:
	int type;

	virtual ~AbstractNode() {};
	// virtual void save(std::ofstream& output_file,
	// 				  int scope_id,
	// 				  int scope_index) = 0;
	// virtual void save_for_display(std::ofstream& output_file) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */