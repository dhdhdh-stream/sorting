#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

#include <map>
#include <set>
#include <utility>
#include <vector>

class ContextLayer {
public:
	int scope_id;
	int node_id;

	std::map<std::pair<std::pair<std::vector<int>,std::vector<int>>,int>, double> obs_history;
};

#endif /* CONTEXT_LAYER_H */