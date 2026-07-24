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

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	double average_max_update;
	int epoch_iter;
	int last_update_iter;

	ActionNetwork(int num_states);
	ActionNetwork(ActionNetwork* original);
	ActionNetwork(std::ifstream& input_file);
	~ActionNetwork();

	void activate(Eigen::VectorXf& state_vals);

	void save(ActionNetworkHistory* history);
	void load(ActionNetworkHistory* history);

	void backprop(Eigen::VectorXf& state_errors);

	void update();

	/**
	 * - for debug
	 */
	void get_max_update(double& max_update_size);

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class ActionNetworkHistory : public AbstractNetworkHistory {
public:
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;
	Eigen::VectorXf output_history;

	ActionNetworkHistory(ActionNetwork* network);
};

#endif /* ACTION_NETWORK_H */