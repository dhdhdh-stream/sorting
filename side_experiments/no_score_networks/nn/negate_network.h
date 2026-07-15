/**
 * - training (away from -1.0) leads to instability(?)
 *   - and creates strong dependencies that can make it difficult to generalize(?)
 */

#ifndef NEGATE_NETWORK_H
#define NEGATE_NETWORK_H

#include <fstream>
#include <vector>

#include "abstract_network.h"

class NegateNetworkHistory;
class NegateNetwork : public AbstractNetwork {
public:
	int state;

	NegateNetwork(int state);
	NegateNetwork(NegateNetwork* original);
	NegateNetwork(std::ifstream& input_file);

	void activate(std::vector<double>& state_vals);

	void backprop_through(std::vector<double>& state_errors);

	void save(std::ofstream& output_file);
};

class NegateNetworkHistory : public AbstractNetworkHistory {
public:
	NegateNetworkHistory(NegateNetwork* network);
};

#endif /* NEGATE_NETWORK_H */