#ifndef TRANSITION_NETWORK_H
#define TRANSITION_NETWORK_H

#include <Eigen/Dense>

#include "abstract_network.h"
#include "layer.h"

class TransitionNetworkHistory;
class TransitionNetwork : public AbstractNetwork {
public:
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int num_instances;
	int last_update_iter;
	int epoch_iter;

	TransitionNetwork(int front_num_states,
					  int back_num_states);
	TransitionNetwork(TransitionNetwork* original);
	TransitionNetwork(std::ifstream& input_file);
	~TransitionNetwork();

	void activate(Eigen::VectorXf& front_state_vals,
				  Eigen::VectorXf& back_state_vals);

	void save(TransitionNetworkHistory* history);
	void load(TransitionNetworkHistory* history);

	void backprop(Eigen::VectorXf& back_state_errors,
				  Eigen::VectorXf& front_state_errors);

	void update();

	void clear_momentum();

	void add_front_states(int new_num_states);
	void add_back_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class TransitionNetworkHistory : public AbstractNetworkHistory {
public:
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;

	TransitionNetworkHistory(TransitionNetwork* network);
};

#endif /* TRANSITION_NETWORK_H */