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
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	FoldNetwork(int flat_size,
				int output_size);
	FoldNetwork(std::ifstream& input_file);
	FoldNetwork(FoldNetwork* original);
	~FoldNetwork();

	void activate(double* flat_inputs,
				  bool* activated,
				  std::vector<double>& obs);
	void backprop(std::vector<double>& errors,
				  double target_max_update);

	void add_scope(int scope_size);
	void pop_scope();
	void reset_last();
	void set_just_score();
	void set_can_compress();
	void activate(double* flat_inputs,
				  bool* activated,
				  std::vector<double>& obs,
				  std::vector<std::vector<double>>& state_vals);
	void backprop_last_state(std::vector<double>& errors,
							 double target_max_update);
	void backprop_full_state(std::vector<double>& errors,
							 double target_max_update);

	void save(std::ofstream& output_file);

private:
	void construct();
};

#endif /* FOLD_NETWORK_H */