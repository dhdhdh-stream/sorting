#ifndef SUB_FOLD_NETWORK_H
#define SUB_FOLD_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class SubFoldNetwork : public AbstractNetwork {
public:
	std::vector<int> scope_sizes;
	std::vector<int> s_input_sizes;
	int output_size;

	int fold_index;

	std::vector<Layer*> state_inputs;
	std::vector<Layer*> s_input_inputs;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	SubFoldNetwork(std::vector<int> scope_sizes,
				   std::vector<int> s_input_sizes,
				   int output_size);
	SubFoldNetwork(SubFoldNetwork* original);
	~SubFoldNetwork();

	void add_s_input(int layer,
					 int num_input);
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<std::vector<double>>& s_input_vals);
	void activate_new_s_input(std::vector<std::vector<double>>& state_vals,
							  std::vector<std::vector<double>>& s_input_vals);
	void backprop_weights_with_no_error_signal(std::vector<double>& errors,
											   double target_max_update);
	void backprop_new_s_input(int layer,
							  int new_input_size,
							  std::vector<double>& errors,
							  double target_max_update);
	// void backprop(std::vector<double>& errors,
	// 			  double target_max_update);

private:
	void construct();
};

#endif /* SUB_FOLD_NETWORK_H */