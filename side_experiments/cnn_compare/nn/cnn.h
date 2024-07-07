/**
 * - quick, hardcoded implementation to compare against GPU implementation
 */

#ifndef CNN_H
#define CNN_H

#include <vector>

#include "layer.h"

class CNN {
public:
	std::vector<std::vector<double>> conv_inputs;
	std::vector<std::vector<double>> weights;
	std::vector<double> constants;
	std::vector<std::vector<double>> weight_updates;
	std::vector<double> constant_updates;

	Layer* input;
	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double conv_average_max_update;
	double hidden_average_max_update;
	double output_average_max_update;

	CNN();
	~CNN();

	void activate(std::vector<std::vector<double>>& input_vals);
	void backprop(double error);
};

#endif /* CNN_H */