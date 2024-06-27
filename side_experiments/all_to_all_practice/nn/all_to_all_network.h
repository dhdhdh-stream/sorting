// - this doesn't make sense actually
//   - there are many obs that would try to learn which are useless
//     - which prevents what is actually relevant from being learned

// - only pay attention to obs that are relevant
//   - used in branch nodes/eval

// - instead just use flat

#ifndef ALL_TO_ALL_NETWORK_H
#define ALL_TO_ALL_NETWORK_H

#include <vector>

#include "layer.h"

class AllToAllNetwork {
public:
	Layer* input;
	Layer* is_output_input;
	Layer* input_to_hidden;
	Layer* hidden;
	Layer* hidden_to_output;
	Layer* output;

	std::vector<double> output_average_errors;

	int epoch_iter;
	double input_to_hidden_average_max_update;
	double hidden_average_max_update;
	double hidden_to_output_average_max_update;
	double output_average_max_update;

	AllToAllNetwork();
	~AllToAllNetwork();

	void activate(std::vector<double>& input_vals,
				  std::vector<bool>& is_output);
	void backprop(std::vector<double>& errors,
				  std::vector<bool>& is_output);
};

#endif /* ALL_TO_ALL_NETWORK_H */