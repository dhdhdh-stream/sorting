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

	double best_surprise;

	std::vector<std::vector<ScopeHistory*>> post_scope_histories;
	double consistency;

	ExploreInstance();
	~ExploreInstance();

	void calc_consistency();

	void print();
};

#endif /* EXPLORE_INSTANCE_H */