#ifndef EXPLORE_H
#define EXPLORE_H

#include <vector>

class AbstractNode;
class Problem;
class Scope;
class SolutionWrapper;

class Explore {
public:
	Scope* scope_context;
	AbstractNode* node_context;
	bool is_branch;

	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	Explore(Scope* scope_context,
			AbstractNode* node_context,
			bool is_branch,
			SolutionWrapper* wrapper);

	void check_activate(SolutionWrapper* wrapper);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper);
	void set_action(int action,
					SolutionWrapper* wrapper);
	void explore_exit_step(SolutionWrapper* wrapper);
};

class ExploreState {
public:
	Explore* explore;

	int step_index;

	ExploreState(Explore* explore);
};

#endif /* EXPLORE_H */