#ifndef ALL_TO_ALL_NETWORK_H
#define ALL_TO_ALL_NETWORK_H

#include <vector>

#include "layer.h"

class AllToAllNetwork {
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

	AllToAllNetwork(int num_inputs);
	AllToAllNetwork(std::ifstream& input_file);
	~AllToAllNetwork();

	void activate(std::vector<double>& input_vals);
	void backprop(std::vector<double>& errors);

	void save(std::ofstream& output_file);
};

#endif /* ALL_TO_ALL_NETWORK_H */