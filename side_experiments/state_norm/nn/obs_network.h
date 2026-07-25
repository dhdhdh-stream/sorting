/**
 * - do not individually normalize obs for each network
 *   - unstable when generalizing
 */

#ifndef OBS_NETWORK_H
#define OBS_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "abstract_network.h"
#include "layer.h"

class ObsNetworkHistory;
class ObsNetwork : public AbstractNetwork {
public:
	Eigen::VectorXf state_norms;
	Layer* state_input;

	Layer* obs_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int run_num_instances;
	int last_get_max_update_iter;
	int last_update_weights_iter;

	ObsNetwork(int num_states,
			   int num_obs);
	ObsNetwork(ObsNetwork* original);
	ObsNetwork(std::ifstream& input_file);
	~ObsNetwork();

	void activate(Eigen::VectorXf& state_norms,
				  Eigen::VectorXf& state_vals,
				  std::vector<double>& obs_input_vals);
	void activate_w_drop(Eigen::VectorXf& state_norms,
						 Eigen::VectorXf& state_vals,
						 std::vector<double>& obs_input_vals);

	void save(ObsNetworkHistory* history);
	void load(ObsNetworkHistory* history);

	void backprop(Eigen::VectorXf& state_errors);

	void get_max_update(double& max_update_size);
	void update_weights(double learning_rate);

	void clear_update_weights();

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class ObsNetworkHistory : public AbstractNetworkHistory {
public:
	Eigen::VectorXf state_norms_history;
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf obs_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;
	Eigen::VectorXf output_history;

	ObsNetworkHistory(ObsNetwork* network);
};

#endif /* OBS_NETWORK_H */