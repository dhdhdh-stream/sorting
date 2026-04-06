#ifndef FOCUS_NETWORK_H
#define FOCUS_NETWORK_H

#include <vector>

#include "layer.h"

class FocusNetwork {
public:
	std::vector<int> input_indexes;

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

	FocusNetwork(std::vector<int>& input_indexes);
	FocusNetwork(FocusNetwork* original);
	FocusNetwork(std::ifstream& input_file);
	~FocusNetwork();

	void activate(std::vector<double>& input_vals);
	void backprop(double error);

	void save(std::ofstream& output_file);
};

#endif /* FOCUS_NETWORK_H */