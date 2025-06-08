#ifndef CONFUSION_H
#define CONFUSION_H

#include <vector>

class AbstractNode;
class Problem;
class Scope;
class SolutionWrapper;

class Confusion {
public:
	Scope* scope_context;
	AbstractNode* node_context;
	bool is_branch;

	int iter;

	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	Confusion(Scope* scope_context,
			  AbstractNode* node_context,
			  bool is_branch,
			  SolutionWrapper* wrapper);

	void check_activate(SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);
};

class ConfusionState {
public:
	Confusion* confusion;

	int step_index;

	ConfusionState(Confusion* confusion);
};

#endif /* CONFUSION_H */