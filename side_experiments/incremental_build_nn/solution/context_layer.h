#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class ContextLayer {
public:
	Scope* scope;
	ScopeNode* node;

	ScopeHistory* scope_history;
};

#endif /* CONTEXT_LAYER_H */