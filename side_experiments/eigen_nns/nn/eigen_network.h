#ifndef EIGEN_NETWORK_H
#define EIGEN_NETWORK_H

#include <vector>

#include "eigen_layer.h"

class EigenNetwork {
public:
	EigenLayer* input;

	EigenLayer* hidden_1;
	EigenLayer* hidden_2;
	EigenLayer* hidden_3;
	EigenLayer* output;

	int epoch_iter;
	double average_max_update;

	EigenNetwork(int input_size,
				 int output_size);
	EigenNetwork(EigenNetwork* original);
	EigenNetwork(std::ifstream& input_file);
	~EigenNetwork();

	void activate(std::vector<double>& input_vals);
	void backprop(std::vector<double>& errors);

	void save(std::ofstream& output_file);
};

#endif /* EIGEN_NETWORK_H */