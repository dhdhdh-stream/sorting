#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "layer.h"

class Network {
public:
	std::vector<double> input_averages;
	std::vector<double> input_standard_deviations;
	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* hidden_3;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double hidden_3_average_max_update;
	double output_average_max_update;

	Network(int input_size,
			std::vector<std::vector<double>>& sample_inputs,
			std::vector<std::vector<bool>>& sample_is_on);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& input_vals,
				  std::vector<bool>& input_is_on);
	void backprop(double error);

	void remove_input(int index);
	/**
	 * - when removing, don't worry about whether removing early or near
	 *   - either can be good, leading to different follow-ups
	 *   - difficult to control when also trying to remove inputs
	 */

	void save(std::ofstream& output_file);
};

#endif /* NETWORK_H */