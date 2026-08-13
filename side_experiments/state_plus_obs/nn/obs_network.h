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
	Layer* state_input;

	Layer* obs_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int num_instances;
	int last_update_iter;
	int epoch_iter;

	ObsNetwork(int num_states,
			   int num_obs,
			   double multiplier);
	ObsNetwork(ObsNetwork* original);
	ObsNetwork(std::ifstream& input_file);
	~ObsNetwork();

	void activate(Eigen::VectorXf& state_vals,
				  std::vector<double>& obs_input_vals);

	void save(ObsNetworkHistory* history);
	void load(ObsNetworkHistory* history);

	void backprop(Eigen::VectorXf& state_errors);

	void update();

	void clear_momentum();

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class ObsNetworkHistory : public AbstractNetworkHistory {
public:
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf obs_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;

	ObsNetworkHistory(ObsNetwork* network);
};

#endif /* OBS_NETWORK_H */