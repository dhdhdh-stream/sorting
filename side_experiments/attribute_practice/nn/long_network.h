#ifndef LONG_NETWORK_H
#define LONG_NETWORK_H

#include <vector>

#include "layer.h"

class LongNetwork {
public:
	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* hidden_3;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double hidden_3_average_max_update;
	double output_average_max_update;

	LongNetwork(int input_size);
	LongNetwork(LongNetwork* original);
	LongNetwork(std::ifstream& input_file);
	~LongNetwork();

	void activate(std::vector<double>& input_vals);
	void backprop(double error);

	void save(std::ofstream& output_file);
};

#endif /* LONG_NETWORK_H */