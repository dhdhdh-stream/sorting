#ifndef SHALLOW_DUPLICATE_NETWORK_H
#define SHALLOW_DUPLICATE_NETWORK_H

#include <vector>

#include "layer.h"

class ShallowDuplicateNetwork {
public:
	Layer* input;
	Layer* is_on_input;
	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	ShallowDuplicateNetwork();
	~ShallowDuplicateNetwork();

	void activate(std::vector<double>& input_vals,
				  std::vector<bool>& is_on);
	void backprop(double error);

};

#endif /* SHALLOW_DUPLICATE_NETWORK_H */