#ifndef FAMILIARITY_NETWORK_H
#define FAMILIARITY_NETWORK_H

#include <vector>

#include "layer.h"

class FamiliarityNetwork {
public:
	Layer* input;
	Layer* hidden_0;
	Layer* hidden_1;
	Layer* output;
	/**
	 * - simply don't separate into val and is_on
	 *   - so predict as a single target, instead of awkwardly as two
	 */

	int epoch_iter;
	double hidden_0_average_max_update;
	double hidden_1_average_max_update;
	double output_average_max_update;

	FamiliarityNetwork(int num_inputs);
	FamiliarityNetwork(FamiliarityNetwork* original);
	FamiliarityNetwork(std::ifstream& input_file);
	~FamiliarityNetwork();

	void activate(std::vector<double>& input);
	void backprop(double error);

	void save(std::ofstream& output_file);
};

#endif /* FAMILIARITY_NETWORK_H */