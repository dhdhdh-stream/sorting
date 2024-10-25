/**
 * - don't use softmax
 *   - does not correspond to probability distribution(?)
 *     - exponential makes predictions way too confident
 */

#ifndef ACTION_NETWORK_H
#define ACTION_NETWORK_H

#include <vector>

#include "layer.h"

class ActionNetworkHistory;
class ActionNetwork {
public:
	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	ActionNetwork(int num_states,
				  int num_actions);
	ActionNetwork(std::ifstream& input_file);
	~ActionNetwork();

	void activate(std::vector<double>& state_vals);
	void activate(std::vector<double>& state_vals,
				  ActionNetworkHistory* history);
	void backprop(int target,
				  std::vector<double>& state_errors,
				  ActionNetworkHistory* history);
	void update_weights();

	void save(std::ofstream& output_file);
};

class ActionNetworkHistory {
public:
	std::vector<double> input_histories;
	std::vector<double> hidden_1_histories;
	std::vector<double> hidden_2_histories;
	std::vector<double> output_histories;
};

#endif /* ACTION_NETWORK_H */