#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class ScopeHistory;

class ContextLayer {
public:
	int scope_id;
	int node_id;

	/**
	 * - actual copy in ScopeNode
	 */
	std::vector<double>* state_vals;
	std::vector<double> state_weights;

	ScopeHistory* scope_history;
};

#endif /* CONTEXT_LAYER_H */