#ifndef FOLD_NETWORK_H
#define FOLD_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "fold_helper.h"
#include "layer.h"
#include "node.h"

class FoldNetwork {
public:
	std::vector<Node*> input_mappings;

	int state_size;

	Layer* flat_input;
	Layer* activated_input;
	Layer* state_input;

	Layer* hidden;
	Layer* output;

	int epoch;
	int iter;

	std::mutex mtx;

	FoldNetwork(std::vector<Node*> input_mappings,
				int state_size);
	FoldNetwork(std::vector<Node*> input_mappings,
				int state_size,
				std::ifstream& input_file);
	~FoldNetwork();

	void activate(std::vector<double>& inputs,
				  std::vector<bool>& activated,
				  double* state_vals);
	void full_backprop(std::vector<double>& errors,
					   double* state_errors);
	void full_calc_max_update(double& max_update,
							  double learning_rate,
							  double momentum);
	void full_update_weights(double factor,
							 double learning_rate,
							 double momentum);

	void state_backprop(std::vector<double>& errors,
						double* state_errors);
	void state_calc_max_update(double& max_update,
							   double learning_rate,
							   double momentum);
	void state_update_weights(double factor,
							  double learning_rate,
							  double momentum);

	void save(std::ofstream& output_file);
};

#endif /* FOLD_NETWORK_H */