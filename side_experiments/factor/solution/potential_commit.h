#ifndef POTENTIAL_COMMIT_H
#define POTENTIAL_COMMIT_H

#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class Problem;

class PotentialCommit {
public:
	AbstractNode* node_context;
	bool is_branch;

	std::vector<int> step_types;
	std::vector<Action*> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void finalize();
};

#endif /* POTENTIAL_COMMIT_H */