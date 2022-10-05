#ifndef FOLD_LOOP_INIT_NETWORK_H
#define FOLD_LOOP_INIT_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class FoldLoopInitNetwork : public AbstractNetwork {
public:
	std::vector<int> pre_loop_flat_sizes;
	int loop_state_size;

	std::vector<Layer*> pre_loop_flat_inputs;

	int outer_fold_index;

	std::vector<int> scope_sizes;
	std::vector<Layer*> state_inputs;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	FoldLoopInitNetwork(std::vector<int> pre_loop_flat_sizes,
						int loop_state_size);
	FoldLoopInitNetwork(std::ifstream& input_file);
	FoldLoopInitNetwork(FoldLoopInitNetwork* original);
	~FoldLoopInitNetwork();

	void activate(std::vector<std::vector<double>>& pre_loop_flat_vals);
	void backprop(std::vector<double>& errors,
				  double target_max_update);

	void add_scope(int scope_size);
	void pop_scope();
	void reset_last();
	void init_activate(std::vector<std::vector<double>>& pre_loop_flat_vals,
						std::vector<std::vector<double>>& state_vals);
	void backprop_last_state(std::vector<double>& errors,
							 double target_max_update);
	void backprop_full_state(std::vector<double>& errors,
							 double target_max_update);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class FoldLoopInitNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<std::vector<double>> pre_loop_flat_inputs_historys;

	std::vector<std::vector<double>> state_inputs_historys;

	std::vector<double> hidden_history;
	std::vector<double> output_history;

	FoldLoopInitNetworkHistory(FoldLoopInitNetwork* network);
	void reset_weights() override;
};

#endif /* FOLD_LOOP_INIT_NETWORK_H */