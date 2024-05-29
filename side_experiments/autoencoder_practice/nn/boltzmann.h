#ifndef BOLTZMANN_H
#define BOLTZMANN_H

#include <vector>

#include "layer.h"

class Boltzmann {
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

	Boltzmann();
	~Boltzmann();

	void activate(std::vector<double>& input_vals);
	void backprop(std::vector<double>& errors);

	int classify(std::vector<double>& input_vals);
};

#endif /* BOLTZMANN_H */