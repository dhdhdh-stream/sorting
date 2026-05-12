#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "layer.h"

class Network {
public:
	int type;

	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	Network(int type,
			int input_size,
			int output_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& input_vals);
	void backprop(std::vector<double>& errors);

	// temp
	void update();
	void update(double target_max_update);

	void save(std::ofstream& output_file);
};

#endif /* NETWORK_H */