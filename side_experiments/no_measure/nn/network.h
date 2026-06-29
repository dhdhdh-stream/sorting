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

	int epoch_iter;
	double average_max_update;

	Network(int input_size,
			std::vector<double>& init_means,
			std::vector<double>& init_deviations);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& input_vals);

	void init_backprop(double error,
					   double& hidden_1_average_max_update,
					   double& hidden_2_average_max_update,
					   double& hidden_3_average_max_update,
					   double& output_average_max_update);

	void backprop(double error);
	void update();

	void save(std::ofstream& output_file);
};

#endif /* NETWORK_H */