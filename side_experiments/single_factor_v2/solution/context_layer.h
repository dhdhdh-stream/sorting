#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class ContextLayer {
public:
	int scope_id;
	int node_id;

	std::vector<bool> state_on;
	std::vector<double> state_vals;

	std::vector<bool> score_state_on;
	std::vector<double> score_state_vals;

	ScopeHistory* scope_history;

	
};

#endif /* CONTEXT_LAYER_H */