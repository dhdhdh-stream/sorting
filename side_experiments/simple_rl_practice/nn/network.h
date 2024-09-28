#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "layer.h"

class Network {
public:
	Layer* input;
	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	Network(int num_obs);
	~Network();

	void activate(std::vector<double>& obs_vals);
	void backprop(std::vector<double>& obs_vals,
				  double eval);
};

#endif /* NETWORK_H */