#ifndef CONFUSION_H
#define CONFUSION_H

#include <vector>

#include "action.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;

class Confusion {
public:
	Scope* scope_context;
	AbstractNode* node_context;
	bool is_branch;

	int iter;

	std::vector<int> step_types;
	std::vector<Action> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	Confusion(Scope* scope_context,
			  AbstractNode* node_context,
			  bool is_branch);

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper);
};

#endif /* CONFUSION_H */