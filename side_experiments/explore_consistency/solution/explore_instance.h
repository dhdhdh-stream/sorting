#ifndef EXPLORE_INSTANCE_H
#define EXPLORE_INSTANCE_H

#include <vector>

class AbstractNode;
class ExploreExperiment;
class Scope;
class ScopeHistory;

class ExploreInstance {
public:
	ExploreExperiment* experiment;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	std::vector<AbstractNode*> new_nodes;

	double surprise;

	ScopeHistory* scope_history;
	double consistency;

	ExploreInstance();
	~ExploreInstance();
};

#endif /* EXPLORE_INSTANCE_H */