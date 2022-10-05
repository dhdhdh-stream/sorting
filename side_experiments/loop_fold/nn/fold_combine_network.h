#ifndef FOLD_COMBINE_NETWORK_H
#define FOLD_COMBINE_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class FoldCombineNetwork : public AbstractNetwork {
public:
	int loop_state_size;
	std::vector<int> pre_loop_flat_sizes;
	std::vector<int> post_loop_flat_sizes;

	Layer* loop_state_input;
	std::vector<Layer*> pre_loop_flat_inputs;
	std::vector<Layer*> post_loop_flat_inputs;

	int outer_fold_index;
	double average_error;

	std::vector<int> scope_sizes;
	std::vector<Layer*> state_inputs;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	FoldCombineNetwork(int loop_state_size,
					   std::vector<int> pre_loop_flat_sizes,
					   std::vector<int> post_loop_flat_sizes);
	FoldCombineNetwork(std::ifstream& input_file);
	FoldCombineNetwork(FoldCombineNetwork* original);
	~FoldCombineNetwork();

	void activate(std::vector<double>& loop_state,
				  std::vector<std::vector<double>>& pre_loop_flat_vals,
				  std::vector<std::vector<double>>& post_loop_flat_vals);
	void backprop(std::vector<double>& errors,
				  double target_max_update);

	void add_scope(int scope_size);
	void pop_scope();
	void reset_last();
	void init_activate(std::vector<double>& loop_state,
					   std::vector<std::vector<double>>& pre_loop_flat_vals,
					   std::vector<std::vector<double>>& post_loop_flat_vals,
					   std::vector<std::vector<double>>& state_vals);
	void loop_activate(std::vector<double>& loop_state,
					   std::vector<std::vector<double>>& post_loop_flat_vals,
					   std::vector<std::vector<double>>& state_vals);
	void combine_activate(std::vector<std::vector<double>>& post_loop_flat_vals,
						  std::vector<std::vector<double>>& state_vals);
	void backprop_last_state(std::vector<double>& errors,
							 double target_max_update);
	void backprop_full_state(std::vector<double>& errors,
							 double target_max_update);
	void backprop_loop_errors_with_no_weight_change(std::vector<double>& errors);

	void save(std::ofstream& output_file);

private:
	void construct();
};

#endif /* FOLD_COMBINE_NETWORK_H */