#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_INNER_SCOPE = 1;
const int NODE_TYPE_BRANCH = 2;
const int NODE_TYPE_SCOPE_EXIT = 3;

// TODO: remove fold nodes
// - track experiments in action nodes and scope nodes
//   - assign target node if run passed without any experiment being hit

class AbstractNode {
public:
	int type;

	Scope* parent;
	int id;

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