#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class StateNetwork : public AbstractNetwork {
public:
	std::vector<int> scope_sizes;
	std::vector<Layer*> state_inputs;
	Layer* scopes_on_input;
	Layer* obs_input;

	Layer* hidden;
	Layer* output;

	std::mutex mtx;

	StateNetwork(std::vector<int> scope_sizes);
	StateNetwork(std::ifstream& input_file);
	~StateNetwork();

	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  std::vector<double>& obs);
	void backprop(std::vector<double>& errors);
	void backprop_weights_with_no_error_signal(std::vector<double>& errors);
	void calc_max_update(double& max_update,
						 double learning_rate);
	void update_weights(double factor,
						double learning_rate);

	void save(std::ofstream& output_file);

private:
	void construct();
};

#endif /* STATE_NETWORK_H */