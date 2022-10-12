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
	int output_size;

	int fold_index;

	std::vector<Layer*> state_inputs;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	SubFoldNetwork(std::vector<int> scope_sizes,
				  int output_size);
	SubFoldNetwork(std::ifstream& input_file);
	SubFoldNetwork(SubFoldNetwork* original);
	~SubFoldNetwork();

	void add_state(int layer,
				   int num_state);
	void activate(std::vector<std::vector<double>>& state_vals);
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop_weights_with_no_error_signal(std::vector<double>& errors,
											   double target_max_update);
	// void backprop_new_state(int layer,
	// 						int new_input_size,
	// 						std::vector<double>& errors,
	// 						double target_max_update);
	void backprop(std::vector<double>& errors,
				  double target_max_update);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class SubFoldNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<std::vector<double>> state_inputs_historys;

	std::vector<double> hidden_history;
	std::vector<double> output_history;

	SubFoldNetworkHistory(SubFoldNetwork* network);
	void reset_weights() override;
};

#endif /* SUB_FOLD_NETWORK_H */