#ifndef INIT_NETWORK_H
#define INIT_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "abstract_network.h"
#include "layer.h"

class InitNetworkHistory;
class InitNetwork : public AbstractNetwork {
public:
	std::vector<int> init_states;

	Layer* state_input;

	Layer* obs_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	bool is_ramp;

	int num_instances;
	int last_update_iter;
	int epoch_iter;

	InitNetwork(std::vector<int>& init_states,
				int num_states,
				int num_obs);
	InitNetwork(InitNetwork* original);
	InitNetwork(std::ifstream& input_file);
	~InitNetwork();

	void init_activate(Eigen::VectorXf& state_vals,
					   std::vector<double>& new_state_vals,
					   std::vector<double>& obs_input_vals);
	void init_backprop(std::vector<double>& new_state_errors);
	void init_update();

	void activate(Eigen::VectorXf& state_vals,
				  std::vector<double>& obs_input_vals);

	void save(InitNetworkHistory* history);
	void load(InitNetworkHistory* history);

	void backprop(Eigen::VectorXf& state_errors);

	void update();

	void clear_momentum();

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class InitNetworkHistory : public AbstractNetworkHistory {
public:
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf obs_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;

	InitNetworkHistory(InitNetwork* network);
};

#endif /* INIT_NETWORK_H */