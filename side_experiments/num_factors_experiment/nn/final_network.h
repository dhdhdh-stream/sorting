#ifndef FINAL_NETWORK_H
#define FINAL_NETWORK_H

#include <vector>

#include "layer.h"

class FinalNetwork {
public:
	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	FinalNetwork(int input_size);
	FinalNetwork(FinalNetwork* original);
	FinalNetwork(std::ifstream& input_file);
	~FinalNetwork();

	void activate(std::vector<double>& input_vals);
	void backprop(double error);

	// temp
	void update();
	void update(double target_max_update);

	void save(std::ofstream& output_file);
};

#endif /* FINAL_NETWORK_H */