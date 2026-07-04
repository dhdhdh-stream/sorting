#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "layer.h"

class NetworkHistory;
class Network {
public:
	Layer* raw_input;
	Eigen::VectorXf input_means;
	Eigen::VectorXf input_deviations;
	/**
	 * - just to help network initialize
	 * - calculate once on init
	 * 
	 * - do not normalize inner
	 *   - gradually weakens signals
	 *   - if mean/deviation gets large enough, normalization can outpace any possible adjustment
	 *     - permanently destroying signal
	 */
	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* hidden_3;
	Layer* output;

	Network(int input_size,
			std::vector<double>& init_means,
			std::vector<double>& init_deviations,
			int output_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& input_vals);
	void activate(std::vector<double>& input_vals,
				  NetworkHistory* history);

	void backprop(std::vector<double>& errors);
	void backprop(std::vector<double>& errors,
				  NetworkHistory* history);

	void init_update(double& hidden_1_average_max_update,
					 double& hidden_2_average_max_update,
					 double& hidden_3_average_max_update,
					 double& output_average_max_update);

	void get_max_update(double& max_update);
	void update_weights(double learning_rate);

	void save(std::ofstream& output_file);
};

class NetworkHistory {
public:
	std::vector<double> input_history;
	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;
	std::vector<double> hidden_3_history;
};

#endif /* NETWORK_H */