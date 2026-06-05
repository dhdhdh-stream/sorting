#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

#include <vector>

#include "layer.h"

class StateNetworkHistory;
class StateNetwork {
public:
	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	StateNetwork(int input_size,
				 int output_size);
	StateNetwork(StateNetwork* original);
	StateNetwork(std::ifstream& input_file);
	~StateNetwork();

	void activate(std::vector<double>& input_vals);
	void backprop(std::vector<double>& errors);

	void activate(std::vector<double>& input_vals,
				  StateNetworkHistory* history);
	void backprop(std::vector<double>& errors,
				  StateNetworkHistory* history);

	void get_max_update(double& max_update);
	void update_weights(double learning_rate);

	void add_inputs(int num_add);
	void remove_inputs(int num_remove);
	void add_outputs(int num_add);
	void remove_outputs(int num_remove);

	void save(std::ofstream& output_file);
};

class StateNetworkHistory {
public:
	std::vector<double> input_history;
	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;
};

#endif /* STATE_NETWORK_H */