#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class ForwardContextLayer {
public:
	int scope_id;
	int node_id;

	ScopeHistory* scope_history;

	std::vector<double> state_vals;
	std::vector<StateDefinition*> state_types;	// set to NULL if not initialized
};

class BackwardContextLayer {
public:
	std::vector<double>* state_errors;


};

#endif /* CONTEXT_LAYER_H */