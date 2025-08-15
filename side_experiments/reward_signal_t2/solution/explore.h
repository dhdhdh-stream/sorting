#ifndef EXPLORE_H
#define EXPLORE_H

#include <vector>

class AbstractNode;
class Scope;

class Explore {
public:
	AbstractNode* explore_node;
	bool explore_is_branch;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	Explore();
	~Explore();
};

#endif /* EXPLORE_H */