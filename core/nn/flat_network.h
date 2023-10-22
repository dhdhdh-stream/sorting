#ifndef FLAT_NETWORK_H
#define FLAT_NETWORK_H

#include <vector>

#include "layer.h"

/**
 * - enough to solve 4-way XORs
 *   - needed size scales exponentially with the size of XOR wished to solve
 *     - solving XORs requires weights to align, so extra size gives extra chances for that to happen
 */
const int FLAT_NETWORK_HIDDEN_SIZE = 20;

class FlatNetwork {
public:
	int num_inputs;
	Layer* input;

	Layer* hidden;

	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	FlatNetwork(int num_inputs);
	~FlatNetwork();

	void activate();
	void backprop(double error);
};

#endif /* FLAT_NETWORK_H */