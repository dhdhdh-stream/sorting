#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "layer.h"

class NetworkHistory;
class Network {
public:
	int type;

	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	Network(int type,
			int input_size,
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

	// temp
	void update();
	void update(double target_max_update);

	void add_inputs(int num_add);

	void save(std::ofstream& output_file);
};

class NetworkHistory {
public:
	std::vector<double> input_history;
	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;
};

#endif /* NETWORK_H */