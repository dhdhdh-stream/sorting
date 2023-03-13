#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_INNER_SCOPE = 1;
const int NODE_TYPE_BRANCH = 2;
const int NODE_TYPE_FOLD_SCORE = 3;
const int NODE_TYPE_FOLD_SEQUENCE = 4;
const int NODE_TYPE_PASS_THROUGH = 5;

class AbstractNode {
public:
	int type;

	virtual ~AbstractNode() {};
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	int scope_index;	// index within parent scope

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */