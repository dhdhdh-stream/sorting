#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

#include <map>
#include <utility>
#include <vector>

class AbstractNode;
class AbstractScopeHistory;
class BranchNode;
class Scope;

class ContextLayer {
public:
	Scope* scope;
	AbstractNode* node;

	std::vector<AbstractNode*> branch_nodes_seen;
	std::map<AbstractNode*, std::pair<int,int>> location_history;

	/**
	 * - for NewActionExperiment
	 */
	std::vector<std::pair<AbstractNode*,bool>> nodes_seen;
};

#endif /* CONTEXT_LAYER_H */