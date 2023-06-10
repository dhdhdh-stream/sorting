#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class ContextLayer {
public:
	int scope_id;
	int node_id;

	std::vector<double>* state_vals;
	std::vector<bool>* state_initialized;
	std::vector<TypeDefinition*>* state_types;

	ScopeHistory* scope_history;
};

#endif /* CONTEXT_LAYER_H */