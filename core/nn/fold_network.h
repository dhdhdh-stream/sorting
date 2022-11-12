#ifndef FOLD_NETWORK_H
#define FOLD_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class FoldNetwork : public AbstractNetwork {
public:
	std::vector<int> flat_sizes;
	int output_size;

	std::vector<Layer*> flat_inputs;

	int s_input_size;
	Layer* s_input_input;
	std::vector<int> scope_sizes;	// layer 0 is outer_state
	std::vector<Layer*> state_inputs;

	int hidden_size;
	Layer* hidden;

	Layer* output;

	int fold_index;
	int subfold_index;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	FoldNetwork(std::vector<int> flat_sizes,
				int output_size,
				int outer_s_input_size,
				int outer_state_size,
				int hidden_size);
	FoldNetwork(int output_size,
				int outer_s_input_size,
				std::vector<int> scope_sizes,
				int hidden_size);
	FoldNetwork(FoldNetwork* original);
	FoldNetwork(std::ifstream& input_file);
	~FoldNetwork();

	void activate(std::vector<std::vector<double>>& flat_vals,
				  std::vector<double>& outer_s_input_vals,
				  std::vector<double>& outer_state_vals);
	void backprop(std::vector<double>& errors,
				  double target_max_update);

	void add_scope(int scope_size);
	void pop_scope();
	void activate_fold(std::vector<std::vector<double>>& flat_vals,
					   std::vector<double>& outer_s_input_vals,
					   std::vector<std::vector<double>>& state_vals);
	void backprop_fold_no_state(std::vector<double>& errors,
								double target_max_update);
	void backprop_fold_last_state(std::vector<double>& errors,
								  double target_max_update);

	void set_s_input_size(int s_input_size);
	void activate_subfold(std::vector<double>& s_input_vals,
						  std::vector<std::vector<double>>& state_vals);
	void backprop_subfold(std::vector<double>& errors,
						  double target_max_update);
	void backprop_subfold_weights_with_no_error_signal(std::vector<double>& errors,
													   double target_max_update);
	void backprop_subfold_new_s_input(std::vector<double>& errors,
									  double target_max_update);

	void activate_small(std::vector<double>& s_input_vals,
						std::vector<double>& state_vals);
	void backprop_small(std::vector<double>& errors,
						double target_max_update,
						std::vector<double>& s_input_errors,
						std::vector<double>& state_errors);

	void save(std::ofstream& output_file);

private:
	void construct();
};

#endif /* FOLD_NETWORK_H */