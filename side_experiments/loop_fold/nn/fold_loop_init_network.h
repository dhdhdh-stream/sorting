#ifndef FOLD_LOOP_INIT_NETWORK_H
#define FOLD_LOOP_INIT_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class FoldLoopInitNetwork : public AbstractNetwork {
public:
	int pre_loop_flat_size;
	int loop_flat_size;
	int post_loop_flat_size;
	int obs_size;
	int loop_state_size;

	Layer* pre_loop_flat_input;
	Layer* post_loop_flat_input;
	Layer* obs_input;

	int fold_index;
	double average_error;

	std::vector<int> scope_sizes;
	std::vector<Layer*> state_inputs;

	Layer* hidden;
	Layer* loop_state_output;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double loop_state_output_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	FoldLoopInitNetwork(int pre_loop_flat_size,
						int loop_flat_size,
						int post_loop_flat_size,
						int obs_size,
						int loop_state_size);
	FoldLoopInitNetwork(std::ifstream& input_file);
	FoldLoopInitNetwork(FoldLoopInitNetwork* original);
	~FoldLoopInitNetwork();

	void activate(double* pre_loop_flat_inputs,
				  double* post_loop_flat_inputs,
				  std::vector<double>& obs);
	void activate(double* pre_loop_flat_inputs,
				  double* post_loop_flat_inputs,
				  std::vector<double>& obs,
				  std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop(std::vector<double>& errors,
				  std::vector<double>& loop_state_errors,
				  double target_max_update);

	void add_scope(int scope_size);
	void pop_scope();
	void reset_last();
	void set_just_score();
	void set_can_compress();
	void activate(double* pre_loop_flat_inputs,
				  double* post_loop_flat_inputs,
				  std::vector<double>& obs,
				  std::vector<std::vector<double>>& state_vals);
	void activate(double* pre_loop_flat_inputs,
				  double* post_loop_flat_inputs,
				  std::vector<double>& obs,
				  std::vector<std::vector<double>>& state_vals,
				  std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop_last_state(std::vector<double>& errors,
							 std::vector<double>& loop_state_errors,
							 double target_max_update);
	void backprop_full_state(std::vector<double>& errors,
							 std::vector<double>& loop_state_errors,
							 double target_max_update);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class FoldLoopInitNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<double> pre_loop_flat_input_history;
	std::vector<double> post_loop_flat_input_history;
	std::vector<double> obs_input_history;

	std::vector<std::vector<double>> state_inputs_historys;

	std::vector<double> hidden_history;
	std::vector<double> loop_state_output_history;
	std::vector<double> output_history;

	FoldLoopInitNetworkHistory(FoldLoopInitNetwork* network);
	void reset_weights() override;
};

#endif /* FOLD_LOOP_INIT_NETWORK_H */