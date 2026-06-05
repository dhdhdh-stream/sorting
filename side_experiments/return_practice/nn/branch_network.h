#ifndef BRANCH_NETWORK_H
#define BRANCH_NETWORK_H

#include <vector>

#include "layer.h"

class BranchNetwork {
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

	BranchNetwork(int input_size);
	BranchNetwork(BranchNetwork* original);
	BranchNetwork(std::ifstream& input_file);
	~BranchNetwork();

	void activate(std::vector<double>& input_vals);
	void backprop(double error);

	void add_inputs(int num_add);

	void save(std::ofstream& output_file);
};

#endif /* BRANCH_NETWORK_H */