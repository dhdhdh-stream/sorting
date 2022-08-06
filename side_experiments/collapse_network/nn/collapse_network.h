#ifndef COLLAPSE_NETWORK_H
#define COLLAPSE_NETWORK_H

#include <fstream>
#include <vector>

#include "layer.h"

const int FLAT = 0;
const int COLLAPSE = 1;
const int FOLD = 2;

class CollapseNetwork {
public:
	std::vector<Layer*> time_inputs;
	Layer* global_input;
	
	std::vector<Layer*> collapse_layers;
	std::vector<Layer*> collapse_outputs;

	std::vector<Layer*> fold_layers;
	std::vector<Layer*> fold_outputs;

	Layer* process;
	Layer* output;

	int state;
	int index;

	CollapseNetwork(std::vector<int> time_sizes,
					int global_size);
	CollapseNetwork(CollapseNetwork* original);
	~CollapseNetwork();

	void activate(std::vector<std::vector<double>>& time_vals,
				  std::vector<double>& global_vals);
	void backprop(std::vector<double>& errors);
	void calc_max_update(double& max_update,
						 double learning_rate,
						 double momentum);
	void update_weights(double factor,
						double learning_rate,
						double momentum);

	void next_step(int state_size);
};

#endif /* COLLAPSE_NETWORK_H */