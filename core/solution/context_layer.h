#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class AbstractNode;
class Scope;
class ScopeHistory;

class ContextLayer {
public:
	Scope* scope;
	AbstractNode* node;

	int num_actions;

	ScopeHistory* scope_history;

	ContextLayer() {
		this->num_actions = 0;
	}
};

#endif /* CONTEXT_LAYER_H */