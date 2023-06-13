#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class ContextLayer {
public:
	int scope_id;
	int node_id;

	std::vector<double>* state_vals;
	std::vector<StateDefinition*>* state_types;

	ScopeHistory* scope_history;

	ContextLayer(int scope_id,
				 int node_id,
				 std::vector<double>* state_vals,
				 std::vector<TypeDefinition*>* state_types,
				 ScopeHistory* scope_history) {
		this->scope_id = scope_id;
		this->node_id = node_id;
		this->state_vals = state_vals;
		this->state_types = state_types;
		this->scope_history = scope_history;
	};
};

#endif /* CONTEXT_LAYER_H */