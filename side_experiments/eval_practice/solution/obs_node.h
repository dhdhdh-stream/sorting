#ifndef OBS_NODE_H
#define OBS_NODE_H

#include <vector>

#include "abstract_node.h"

class Problem;
class ScopeHistory;

class ObsNodeHistory;
class ObsNode : public AbstractNode {
public:
	ObsNode();

	void new_activate(Problem* problem,
					  ScopeHistory* scope_history);
};

class ObsNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_history;

	ObsNodeHistory(ObsNode* node);
};

#endif /* OBS_NODE_H */