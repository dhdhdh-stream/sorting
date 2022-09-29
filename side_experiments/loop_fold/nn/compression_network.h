#ifndef COMPRESSION_NETWORK_H
#define COMPRESSION_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class CompressionNetwork : public AbstractNetwork {
public:
	std::vector<int> scope_sizes;
	int output_size;

	std::vector<Layer*> state_inputs;
	Layer* scopes_on_input;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	CompressionNetwork(std::vector<int> scope_sizes,
					   int output_size);
	CompressionNetwork(std::ifstream& input_file);
	CompressionNetwork(CompressionNetwork* original);
	~CompressionNetwork();

	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on);
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop(std::vector<double>& errors,
				  double target_max_update);
	void backprop_weights_with_no_error_signal(std::vector<double>& errors,
											   double target_max_update);
	void get_max_update(double& max_update);
	void update_weights(double learning_rate);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class CompressionNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<std::vector<double>> state_inputs_historys;
	std::vector<double> scopes_on_input_history;

	std::vector<double> hidden_history;
	std::vector<double> output_history;

	CompressionNetworkHistory(CompressionNetwork* network);
	void reset_weights() override;
};

#endif /* COMPRESSION_NETWORK_H */