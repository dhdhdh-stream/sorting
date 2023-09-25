#ifndef FLAT_NETWORK_H
#define FLAT_NETWORK_H

/**
 * - enough to solve 4-way XORs
 *   - needed size scales exponentially with the size of XOR wished to solve
 *     - solving XORs requires weights to align, so extra size gives extra chances for that to happen
 */
const int FLAT_NETWORK_HIDDEN_SIZE = 40;

class FlatNetwork {
public:
	int num_inputs;
	std::vector<Layer*> inputs;

	Layer* hidden;

	Layer* output;


};

#endif /* FLAT_NETWORK_H */