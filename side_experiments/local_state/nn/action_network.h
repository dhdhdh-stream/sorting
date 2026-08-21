#ifndef ACTION_NETWORK_H
#define ACTION_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "layer.h"

class ActionNetworkHistory;
class ActionNetwork {
public:
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int num_instances;
	int last_update_iter;
	int epoch_iter;

	ActionNetwork(int num_states);
	ActionNetwork(ActionNetwork* original);
	ActionNetwork(std::ifstream& input_file);
	~ActionNetwork();

	void activate(Eigen::VectorXf& state_vals);

	void save(ActionNetworkHistory* history);
	void load(ActionNetworkHistory* history);

	void backprop(Eigen::VectorXf& state_errors);
	void backprop_through(Eigen::VectorXf& state_errors);

	void update();

	void clear_momentum();

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class ActionNetworkHistory {
public:
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;
};

#endif /* ACTION_NETWORK_H */