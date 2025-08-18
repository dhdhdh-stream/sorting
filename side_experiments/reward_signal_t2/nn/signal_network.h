#ifndef SIGNAL_NETWORK_H
#define SIGNAL_NETWORK_H

#include <vector>

#include "layer.h"

class SignalNetwork {
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

	SignalNetwork(int input_size);
	SignalNetwork(SignalNetwork* original);
	SignalNetwork(std::ifstream& input_file);
	~SignalNetwork();

	void activate(std::vector<double>& input_vals);
	void backprop(double error);

	void remove_input(int index);

	void save(std::ofstream& output_file);
};

#endif /* SIGNAL_NETWORK_H */