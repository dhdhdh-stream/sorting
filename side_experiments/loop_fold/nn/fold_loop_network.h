#ifndef FOLD_LOOP_NETWORK_H
#define FOLD_LOOP_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class FoldLoopNetwork : public AbstractNetwork {
public:
	int loop_state_size;
	std::vector<int> pre_loop_flat_sizes;
	std::vector<int> loop_flat_sizes;

	Layer* loop_state_input;
	std::vector<Layer*> pre_loop_flat_inputs;
	std::vector<Layer*> loop_flat_inputs;

	int outer_fold_index;
	int inner_fold_index;

	std::vector<int> outer_scope_sizes;
	std::vector<Layer*> outer_state_inputs;

	std::vector<int> inner_scope_sizes;
	std::vector<Layer*> inner_state_inputs;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	FoldLoopNetwork(int loop_state_size,
					std::vector<int> pre_loop_flat_sizes,
					std::vector<int> loop_flat_sizes);
	FoldLoopNetwork(std::ifstream& input_file);
	FoldLoopNetwork(FoldLoopNetwork* original);
	~FoldLoopNetwork();

	void activate(std::vector<double>& loop_state,
				  std::vector<std::vector<double>>& pre_loop_flat_vals,
				  std::vector<std::vector<double>>& loop_flat_vals);
	void activate(std::vector<double>& loop_state,
				  std::vector<std::vector<double>>& pre_loop_flat_vals,
				  std::vector<std::vector<double>>& loop_flat_vals,
				  std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop(std::vector<double>& errors,
				  double target_max_update);

	void outer_add_scope(int scope_size);
	void outer_pop_scope();
	void outer_reset_last();
	void outer_set_just_score();
	void outer_set_can_compress();
	void outer_activate(std::vector<double>& loop_state,
						std::vector<std::vector<double>>& pre_loop_flat_vals,
						std::vector<std::vector<double>>& loop_flat_vals,
						std::vector<std::vector<double>>& outer_state_vals);
	void outer_activate(std::vector<double>& loop_state,
						std::vector<std::vector<double>>& pre_loop_flat_vals,
						std::vector<std::vector<double>>& loop_flat_vals,
						std::vector<std::vector<double>>& outer_state_vals,
						std::vector<AbstractNetworkHistory*>& network_historys);
	void outer_backprop_last_state(std::vector<double>& errors,
								   double target_max_update);
	void outer_backprop_full_state(std::vector<double>& errors,
								   double target_max_update);

	void inner_add_scope(int scope_size);
	void inner_pop_scope();
	void inner_reset_last();
	void inner_set_just_score();
	void inner_set_can_compress();
	void inner_activate(std::vector<double>& loop_state,
						std::vector<std::vector<double>>& loop_flat_vals,
						std::vector<std::vector<double>>& outer_state_vals,
						std::vector<std::vector<double>>& inner_state_vals);
	void inner_activate(std::vector<double>& loop_state,
						std::vector<std::vector<double>>& loop_flat_vals,
						std::vector<std::vector<double>>& outer_state_vals,
						std::vector<std::vector<double>>& inner_state_vals,
						std::vector<AbstractNetworkHistory*>& network_historys);
	void inner_backprop_last_state(std::vector<double>& errors,
								   double target_max_update);
	void inner_backprop_full_state(std::vector<double>& errors,
								   double target_max_update);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class FoldLoopNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<double> loop_state_input_history;
	std::vector<std::vector<double>> pre_loop_flat_inputs_historys;
	std::vector<std::vector<double>> loop_flat_inputs_historys;

	std::vector<std::vector<double>> outer_state_inputs_historys;
	std::vector<std::vector<double>> inner_state_inputs_historys;

	std::vector<double> hidden_history;
	std::vector<double> output_history;

	FoldLoopNetworkHistory(FoldLoopNetwork* network);
	void reset_weights() override;
};

#endif /* FOLD_LOOP_NETWORK_H */