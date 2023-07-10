#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class ForwardContextLayer {
public:
	int scope_id;
	int node_id;

	std::vector<double>* state_vals;	// actual copy in ScopeNode
	// doesn't need to be pointers
	std::vector<bool> states_initialized;

	ScopeHistory* scope_history;
};

class BackwardContextLayer {
public:
	std::vector<double>* state_errors;


};

#endif /* CONTEXT_LAYER_H */