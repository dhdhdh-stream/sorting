// some type of normalization is needed

// normalizing by error unstable
// so probably have to just normalize by target mean
// - can normalize by true variance as well probably

#ifndef MULT_SCALE_NETWORK_H
#define MULT_SCALE_NETWORK_H

#include "layer.h"

class MultScaleNetwork {
public:
	Layer* input;

	Layer* hidden;

	Layer* output;
	std::vector<double> output_average_errors;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	MultScaleNetwork();
	~MultScaleNetwork();

	void activate(std::vector<double>& input_vals);
	void backprop(std::vector<double>& errors);
};

#endif /* MULT_SCALE_NETWORK_H */