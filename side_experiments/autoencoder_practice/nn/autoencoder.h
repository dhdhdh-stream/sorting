#ifndef AUTOENCODER_H
#define AUTOENCODER_H

#include <vector>

#include "layer.h"

class Autoencoder {
public:
	Layer* input;

	Layer* input_to_hidden;

	Layer* hidden;

	Layer* hidden_to_output;

	Layer* output;

	int epoch_iter;
	double input_to_hidden_average_max_update;
	double hidden_average_max_update;
	double hidden_to_output_average_max_update;
	double output_average_max_update;

	Autoencoder();
	~Autoencoder();

	void activate(std::vector<double>& input_vals);
	void backprop(std::vector<double>& errors);
};

#endif /* AUTOENCODER_H */