#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class AbstractNode;
class Scope;
class ScopeHistory;

class ContextLayer {
public:
	Scope* scope;
	AbstractNode* node;

	ScopeHistory* scope_history;
};

#endif /* CONTEXT_LAYER_H */