#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

#include <map>
#include <set>
#include <utility>
#include <vector>

class Scope;

class ContextLayer {
public:
	Scope* scope;
	int node_id;

	std::map<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,int>, double> obs_history;
};

#endif /* CONTEXT_LAYER_H */