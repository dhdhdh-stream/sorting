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

	Layer* raw_obs_input;
	Eigen::VectorXf obs_input_means;
	Eigen::VectorXf obs_input_deviations;
	/**
	 * - to help network initialize
	 *   - but need to constantly update
	 *     - otherwise, if e.g., init_deviation is small, bad when generalized
	 *   - initialize to (0.0, 1.0)
	 *     - if initialize to true, variance can become ~0.0, and cause instability when generalized
	 *     - with 300000 iters of 0.99999 averaging:
	 *       - if true is 0.0, resulting deviation is ~0.5
	 *       - if true is 5.0, resulting deviation is ~4.8
	 *       - if true is 50.0, resulting deviation si ~48.0
	 * 
	 * - do not normalize inner
	 *   - gradually weakens signals
	 *   - if mean/deviation gets large enough, normalization can outpace any possible adjustment
	 *     - permanently destroying signal
	 */
	Layer* obs_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int epoch_iter;
	double average_max_update;
	int last_update_iter;

	InitNetwork(std::vector<int>& init_states,
				int num_states,
				int num_obs);
	InitNetwork(InitNetwork* original);
	InitNetwork(std::ifstream& input_file);
	~InitNetwork();

	void init_activate(std::vector<double>& state_vals,
					   std::vector<double>& new_state_vals,
					   std::vector<double>& obs_input_vals);
	void init_backprop(std::vector<double>& new_state_errors);
	void init_update(double& hidden_1_average_max_update,
					 double& hidden_2_average_max_update,
					 double& output_average_max_update);

	void activate(std::vector<double>& state_vals,
				  std::vector<double>& obs_input_vals);

	void save(InitNetworkHistory* history);
	void load(InitNetworkHistory* history);

	void backprop(std::vector<double>& state_errors);

	void update();

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class InitNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<double> state_input_history;
	std::vector<double> raw_obs_input_history;
	std::vector<double> obs_input_history;
	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;

	InitNetworkHistory(InitNetwork* network);
};

#endif /* INIT_NETWORK_H */