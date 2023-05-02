#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_INNER_SCOPE = 1;
const int NODE_TYPE_BRANCH = 2;
const int NODE_TYPE_FOLD_SCORE = 3;
const int NODE_TYPE_FOLD_SEQUENCE = 4;
const int NODE_TYPE_LOOP_FOLD = 5;
const int NODE_TYPE_PASS_THROUGH = 6;

class AbstractNode {
public:
	int type;

	virtual ~AbstractNode() {};
	virtual void save(std::ofstream& output_file,
					  int scope_id,
					  int scope_index) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	int scope_index;	// index within parent scope

	virtual ~AbstractNodeHistory() {};
	virtual AbstractNodeHistory* deep_copy_for_seed() {
		return NULL;
	};
};

#endif /* ABSTRACT_NODE_H */