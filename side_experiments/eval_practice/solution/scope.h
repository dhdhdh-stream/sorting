#ifndef SCOPE_H
#define SCOPE_H

#include <map>

class AbstractNode;
class AbstractNodeHistory;

class Scope {
public:
	int node_counter;
	std::map<int, AbstractNode*> nodes;

	Scope();
	~Scope();
};

class ScopeHistory {
public:
	Scope* scope;

	std::map<int, AbstractNodeHistory*> node_histories;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */