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

	Layer* input;

	Layer* hidden_1;

	Eigen::VectorXf hidden_1_means;
	Eigen::VectorXf hidden_1_deviations;
	Layer* hidden_1_output;

	Layer* hidden_2;

	Eigen::VectorXf hidden_2_means;
	Eigen::VectorXf hidden_2_deviations;
	Layer* hidden_2_output;

	Layer* hidden_3;

	Eigen::VectorXf hidden_3_means;
	Eigen::VectorXf hidden_3_deviations;
	Layer* hidden_3_output;

	Layer* output;

	int epoch_iter;
	double average_max_update;

	Network(int input_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& input_vals);

	void backprop(double error);
	void update();

	void save(std::ofstream& output_file);
};

#endif /* NETWORK_H */