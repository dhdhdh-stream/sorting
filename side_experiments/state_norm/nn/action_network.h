#ifndef ACTION_NETWORK_H
#define ACTION_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "abstract_network.h"
#include "layer.h"

class ActionNetworkHistory;
class ActionNetwork : public AbstractNetwork {
public:
	Eigen::VectorXf state_norms;
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int run_num_instances;
	int last_get_max_update_iter;
	int last_update_weights_iter;

	ActionNetwork(int num_states);
	ActionNetwork(ActionNetwork* original);
	ActionNetwork(std::ifstream& input_file);
	~ActionNetwork();

	void activate(Eigen::VectorXf& state_norms,
				  Eigen::VectorXf& state_vals);

	void save(ActionNetworkHistory* history);
	void load(ActionNetworkHistory* history);

	void backprop(Eigen::VectorXf& state_errors);

	void get_max_update(double& max_update_size);
	void update_weights(double learning_rate);

	void clear_update_weights();

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class ActionNetworkHistory : public AbstractNetworkHistory {
public:
	Eigen::VectorXf state_norms_history;
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;
	Eigen::VectorXf output_history;

	ActionNetworkHistory(ActionNetwork* network);
};

#endif /* ACTION_NETWORK_H */