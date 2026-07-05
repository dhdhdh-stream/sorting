#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "layer.h"

class Network {
public:
	Layer* raw_input;
	Eigen::VectorXf input_means;
	Eigen::VectorXf input_deviations;
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
	Layer* input;

	// Layer* hidden_1;
	// Layer* hidden_2;
	// Layer* hidden_3;
	// Layer* output;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* hidden_3;
	Layer* hidden_4;
	Layer* hidden_5;
	Layer* output;

	int epoch_iter;
	double average_max_update;

	Network(int input_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& input_vals);

	// void init_backprop(double error,
	// 				   double& hidden_1_average_max_update,
	// 				   double& hidden_2_average_max_update,
	// 				   double& hidden_3_average_max_update,
	// 				   double& output_average_max_update);

	void init_backprop(double error,
					   double& hidden_1_average_max_update,
					   double& hidden_2_average_max_update,
					   double& hidden_3_average_max_update,
					   double& hidden_4_average_max_update,
					   double& hidden_5_average_max_update,
					   double& output_average_max_update);

	void backprop(double error);
	void update();

	void save(std::ofstream& output_file);
};

#endif /* NETWORK_H */