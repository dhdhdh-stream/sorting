#ifndef NETWORK_H
#define NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class NetworkHistory;
class Network : public AbstractNetwork {
public:
	Layer* input;
	Layer* hidden;
	Layer* output;

	int epoch_iter;

	std::mutex mtx;

	Network(int input_size,
			int hidden_size,
			int output_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& vals);
	void activate(std::vector<double>& vals,
				  std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop(std::vector<double>& errors);
	void calc_max_update(double& max_update,
						 double learning_rate);
	void update_weights(double factor,
						double learning_rate);

	void backprop_errors_with_no_weight_change(std::vector<double>& errors);
	void backprop_weights_with_no_error_signal(std::vector<double>& errors);

	void save(std::ofstream& output_file);

private:
	void construct(int input_size,
				   int hidden_size,
				   int output_size);
};

class NetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<double> input_history;
	std::vector<double> hidden_history;
	std::vector<double> output_history;

	NetworkHistory(Network* network);
	void reset_weights() override;
};

#endif /* NETWORK_H */