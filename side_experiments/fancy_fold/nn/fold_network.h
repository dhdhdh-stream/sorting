#ifndef FOLD_NETWORK_H
#define FOLD_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class FoldNetwork : public AbstractNetwork {
public:
	int flat_size;
	int output_size;

	Layer* flat_input;
	Layer* activated_input;
	Layer* obs_input;

	int fold_index;

	std::vector<int> scope_sizes;
	std::vector<Layer*> state_inputs;

	Layer* hidden;
	Layer* output;

	int epoch_iter;

	std::mutex mtx;

	FoldNetwork(int flat_size,
				int output_size);
	FoldNetwork(std::ifstream& input_file);
	FoldNetwork(FoldNetwork* original);
	~FoldNetwork();

	void activate(double* flat_inputs,
				  bool* activated,
				  std::vector<double>& obs);
	void backprop(std::vector<double>& errors);
	void calc_max_update(double& max_update,
						 double learning_rate);
	void update_weights(double factor,
						double learning_rate);

	void add_scope(int scope_size);
	void pop_scope();
	void activate_hidden_linear(double* flat_inputs,
		bool* activated,
		std::vector<double>& obs,
		std::vector<std::vector<double>>& state_vals);
	void backprop_hidden_linear();
	void state_calc_max_update(double& max_update,
							   double learning_rate);
	void state_update_weights(double factor,
							  double learning_rate);
	void activate_full(double* flat_inputs,
					   bool* activated,
					   std::vector<double>& obs,
					   std::vector<std::vector<double>>& state_vals);
	void backprop_full(std::vector<double>& errors);

	void save(std::ofstream& output_file);

private:
	void construct();
};

#endif /* FOLD_NETWORK_H */