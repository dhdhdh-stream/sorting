/**
 * - assume obs already normalized
 *   - or else risk strong signals drowning out weaker correct signals
 *   - don't scale obs to try to bias learning
 *     - instead bias initially when selecting
 */

#ifndef FLAT_NETWORK_H
#define FLAT_NETWORK_H

#include <vector>

/**
 * - select 10 obs
 *   - gives good shot of solving 4-way XOR even including extra loop iters
 *   - anymore risks drowning out correct signals
 */
const int FLAT_NETWORK_OBS_SIZE = 10;

class Layer;

class FlatNetwork {
public:
	std::vector<Layer*> obs_inputs;

	Layer* hidden;

	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	FlatNetwork();
	~FlatNetwork();

	void activate(std::vector<std::vector<double>>& obs_vals);
	void backprop(double output_error);
};

#endif /* FLAT_NETWORK_H */