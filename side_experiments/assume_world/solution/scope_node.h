#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

class Problem;
class Scope;
class Solution;

class ScopeNode : public AbstractNode {
public:
	Scope* scope;

	int next_node_id;
	AbstractNode* next_node;

};

#endif /* SCOPE_NODE_H */