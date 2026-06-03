#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "layer.h"

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int NETWORK_EPOCH_SIZE = 20;

class NetworkHistory;
class Network {
public:
	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	/**
	 * - for init
	 */
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	Network(int input_size,
			int output_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& input_vals);
	void backprop(std::vector<double>& errors);
	void backprop_through(std::vector<double>& errors);

	void activate(std::vector<double>& input_vals,
				  NetworkHistory* history);
	void backprop(std::vector<double>& errors,
				  NetworkHistory* history);
	void backprop_through(std::vector<double>& errors,
						  NetworkHistory* history);

	void update();

	void get_max_update(double& max_update);
	void update_weights(double learning_rate);

	void add_inputs(int num_add);
	void remove_inputs(int num_remove);
	void add_outputs(int num_add);
	void remove_outputs(int num_remove);

	void save(std::ofstream& output_file);
};

class NetworkHistory {
public:
	std::vector<double> input_history;
	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;
};

#endif /* NETWORK_H */