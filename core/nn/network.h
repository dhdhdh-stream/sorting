#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "layer.h"

class Network {
public:
	Layer* input;

	std::vector<Layer*> hiddens;

	Layer* output;

	int epoch_iter;
	std::vector<double> hidden_average_max_updates;
	double output_average_max_update;

	Network();
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& input_vals);
	/**
	 * - don't worry about whether input is on
	 *   - makes it difficult to remove inputs
	 *     - input becomes a proxy for the branch it's on, rather than the value itself
	 */
	void backprop(double error);

	void increment(int new_num_inputs);

	void remove_input(int index);
	/**
	 * - when removing, don't worry about whether removing early or near
	 *   - either can be good, leading to different follow-ups
	 *   - difficult to control when also trying to remove inputs
	 */

	void save(std::ofstream& output_file);
};

#endif /* NETWORK_H */