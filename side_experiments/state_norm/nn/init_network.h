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

	Eigen::VectorXf state_norms;
	Layer* state_input;

	Layer* obs_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	Eigen::VectorXf end_state;

	int last_get_max_update_iter;
	int last_update_weights_iter;

	InitNetwork(std::vector<int>& init_states,
				int num_states,
				int num_obs);
	InitNetwork(InitNetwork* original);
	InitNetwork(std::ifstream& input_file);
	~InitNetwork();

	void init_activate(Eigen::VectorXf& state_norms,
					   Eigen::VectorXf& state_vals,
					   int new_state_norm,
					   std::vector<double>& new_state_vals,
					   std::vector<double>& obs_input_vals);
	void init_activate_w_drop(Eigen::VectorXf& state_norms,
							  Eigen::VectorXf& state_vals,
							  int new_state_norm,
							  std::vector<double>& new_state_vals,
							  std::vector<double>& obs_input_vals);
	void init_backprop(int new_state_norm,
					   std::vector<double>& new_state_errors);
	void init_update(double& hidden_1_average_max_update,
					 double& hidden_2_average_max_update,
					 double& output_average_max_update);

	void activate(Eigen::VectorXf& state_norms,
				  Eigen::VectorXf& state_vals,
				  std::vector<double>& obs_input_vals);
	void activate_w_drop(Eigen::VectorXf& state_norms,
						 Eigen::VectorXf& state_vals,
						 std::vector<double>& obs_input_vals);

	void save(InitNetworkHistory* history);
	void load(InitNetworkHistory* history);

	void backprop(Eigen::VectorXf& state_errors);

	void get_max_update(double& max_update_size);
	void update_weights(double learning_rate);

	void clear_update_weights();

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class InitNetworkHistory : public AbstractNetworkHistory {
public:
	Eigen::VectorXf state_norms_history;
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf obs_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;
	Eigen::VectorXf output_history;

	Eigen::VectorXf end_state_history;

	InitNetworkHistory(InitNetwork* network);
};

#endif /* INIT_NETWORK_H */