#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

class Scope;

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_OBS = 1;

class AbstractNode {
public:
	int type;

	Scope* parent;
	int id;

	virtual ~AbstractNode() {};
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */