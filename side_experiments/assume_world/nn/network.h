/**
 * TODO:
 * - convolution as analyze gets larger
 *   - everytime layer reaches 7x7, add another layer
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "layer.h"

class Network {
public:
	Layer* input;
	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	Network(int analyze_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<std::vector<double>>& input_vals);
	void backprop(double error);

	void save(std::ofstream& output_file);
};

#endif /* NETWORK_H */