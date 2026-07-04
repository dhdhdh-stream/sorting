// TODO: for scope, have start network and end network

#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "layer.h"

class NetworkHistory;
class Network {
public:
	std::vector<Layer*> inputs;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* hidden_3;
	Layer* output;

	Network(std::vector<int>& input_sizes,
			int output_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate();
	void activate(NetworkHistory* history);

	void backprop();
	void backprop(NetworkHistory* history);

	void init_update(double& hidden_1_average_max_update,
					 double& hidden_2_average_max_update,
					 double& hidden_3_average_max_update,
					 double& output_average_max_update);

	void get_max_update(double& max_update);
	void update_weights(double learning_rate);

	void add_inputs(int new_num_states);
	void add_outputs(int new_num_states);

	void save(std::ofstream& output_file);
};

class NetworkHistory {
public:
	Network* network;

	std::vector<std::vector<double>> input_history;
	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;
	std::vector<double> hidden_3_history;

	NetworkHistory(Network* network);
};

#endif /* NETWORK_H */