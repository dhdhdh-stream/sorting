#ifndef SCOPE_NETWORK_H
#define SCOPE_NETWORK_H

class ScopeNetwork {
public:
	std::vector<int> inner_input_indexes;
	Layer* inner_input_input;

	Layer* state_input;

	int hidden_size;
	Layer* hidden_input;

	Layer* output;

	double lasso_weight;
	double impact;

};

#endif /* SCOPE_NETWORK_H */