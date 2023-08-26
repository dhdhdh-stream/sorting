#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class ScopeHistory;

class ForwardContextLayer {
public:
	int scope_id;
	int node_id;

	/**
	 * - actual copy in ScopeNode
	 */
	std::vector<double>* state_vals;
	std::vector<bool> states_initialized;
	/**
	 * - simply always initialize to false, even if unused
	 */
	std::vector<bool> is_learn_existing;

	ScopeHistory* scope_history;
};

class BackwardContextLayer {
public:
	std::vector<double>* state_errors;
};

#endif /* CONTEXT_LAYER_H */