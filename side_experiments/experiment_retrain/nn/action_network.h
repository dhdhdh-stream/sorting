#ifndef ACTION_NETWORK_H
#define ACTION_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "abstract_network.h"
#include "layer.h"

class ActionNetworkHistory;
class ActionNetwork : public AbstractNetwork {
public:
	Layer* state_input;
	Layer* action_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	ActionNetwork(int num_states,
				  int num_actions);
	ActionNetwork(ActionNetwork* original);
	ActionNetwork(std::ifstream& input_file);
	~ActionNetwork();

	void activate(std::vector<double>& state_vals,
				  int action);

	void save(ActionNetworkHistory* history);
	void load(ActionNetworkHistory* history);

	void backprop(std::vector<double>& state_errors);

	void get_max_update(double& max_update);
	void update_weights(double learning_rate);

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class ActionNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<double> state_input_history;
	std::vector<double> action_input_history;
	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;

	ActionNetworkHistory(ActionNetwork* network);
};

#endif /* ACTION_NETWORK_H */