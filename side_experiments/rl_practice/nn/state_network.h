#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

#include <vector>

#include "layer.h"

class StateNetworkHistory;
class StateNetwork {
public:
	Layer* obs_input;
	Layer* action_input;
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	StateNetwork(int num_obs,
				int num_actions,
				int num_state);
	StateNetwork(std::ifstream& input_file);
	~StateNetwork();

	void activate(std::vector<double>& obs_vals,
				  int action,
				  std::vector<double>& state_vals);
	void activate(std::vector<double>& obs_vals,
				  int action,
				  std::vector<double>& state_vals,
				  StateNetworkHistory* history);
	void backprop(std::vector<double>& state_errors,
				  StateNetworkHistory* history);

	void save(std::ofstream& output_file);
};

class StateNetworkHistory {
public:
	std::vector<double> obs_input_history;
	std::vector<double> action_input_history;
	std::vector<double> state_input_history;

	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;
};

#endif /* STATE_NETWORK_H */