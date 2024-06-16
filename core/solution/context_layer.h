#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class AbstractNode;
class AbstractScope;
class AbstractScopeHistory;

class ContextLayer {
public:
	AbstractScope* scope;
	AbstractNode* node;

	AbstractScopeHistory* scope_history;
};

#endif /* CONTEXT_LAYER_H */