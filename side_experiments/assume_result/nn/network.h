/**
 * - even if world fully visible, may be too complex to analyze with a glance
 *   - whether for eval, or for guessing the impact of complex actions
 *   - so is always just best effort
 *     - and limits the impact of world modeling
 * 
 * - due to generalization, will often need to learn in chaotic scenarios
 *   - needs to have sufficient size/capability to handle
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "layer.h"

class Network {
public:
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

	Network(int input_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& input_vals);
	void backprop(double error);

	void save(std::ofstream& output_file);
};

#endif /* NETWORK_H */