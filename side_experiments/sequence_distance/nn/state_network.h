#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

#include <fstream>
#include <vector>

#include "layer.h"

class StateNetworkHistory;
class StateNetwork {
public:
	Layer* obs_input;
	Layer* state_input;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	StateNetwork();
	StateNetwork(std::ifstream& input_file);
	~StateNetwork();

	void activate(double obs_val,
				  double state_val,
				  StateNetworkHistory* history);
	void backprop(double error,
				  StateNetworkHistory* history);

	void activate(double obs_val,
				  double state_val);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class StateNetworkHistory {
public:
	StateNetwork* network;

	double obs_input_history;
	double state_input_history;
	std::vector<double> hidden_history;

	StateNetworkHistory(StateNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* STATE_NETWORK_H */