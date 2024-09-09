#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

#include <map>
#include <set>
#include <utility>
#include <vector>

class AbstractNode;
class Scope;

class ContextLayer {
public:
	Scope* scope;
	AbstractNode* node;

	vector<double> starting_location;

	std::map<AbstractNode*, std::pair<std::vector<double>,std::vector<double>>> history;
	/**
	 * - global location, not relative to starting_location
	 */

	/**
	 * - for NewActionExperiment
	 */
	std::vector<std::pair<AbstractNode*,bool>> nodes_seen;
};

#endif /* CONTEXT_LAYER_H */