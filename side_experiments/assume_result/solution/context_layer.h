#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

#include <map>
#include <set>
#include <utility>
#include <vector>

class AbstractNode;
class ProblemLocation;
class Scope;

class ContextLayer {
public:
	Scope* scope;
	AbstractNode* node;

	ProblemLocation* starting_location;

	std::map<AbstractNode*, ProblemLocation*> location_history;

	/**
	 * - for NewActionExperiment
	 */
	std::vector<std::pair<AbstractNode*,bool>> nodes_seen;
};

#endif /* CONTEXT_LAYER_H */