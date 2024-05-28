#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "layer.h"

class Network {
public:
	Layer* input;

	std::vector<Layer*> hiddens;

	Layer* output;

	int epoch_iter;
	std::vector<double> hidden_average_max_updates;
	double output_average_max_update;

	Network();
	Network(Network* original);
	~Network();

	void activate(std::vector<double>& input_vals);
	void backprop(double error);

	void increment(int new_num_inputs);
};

#endif /* NETWORK_H */