#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

#include <vector>

class AbstractNode;
class AbstractScope;
class AbstractScopeHistory;
class BranchNode;

class ContextLayer {
public:
	AbstractScope* scope;
	AbstractNode* node;

	std::vector<std::pair<AbstractNode*,bool>> nodes_seen;



};

#endif /* CONTEXT_LAYER_H */