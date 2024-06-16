#ifndef ABSTRACT_SCOPE_H
#define ABSTRACT_SCOPE_H

#include <map>

class AbstractNode;
class AbstractNodeHistory;

const int SCOPE_TYPE_SCOPE = 0;
const int SCOPE_TYPE_INFO = 1;

class AbstractScope {
public:
	int type;

	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	virtual ~AbstractScope() {};
};

class AbstractScopeHistory {
public:
	AbstractScope* scope;

	std::map<AbstractNode*, AbstractNodeHistory*> node_histories;

	virtual ~AbstractScopeHistory() {};

	virtual AbstractScopeHistory* deep_copy() = 0;
};

#endif /* ABSTRACT_SCOPE_H */