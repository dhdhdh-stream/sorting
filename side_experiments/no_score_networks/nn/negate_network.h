/**
 * - simply do not train away from -1.0
 *   - drop makes training stable, but still hurts long term results
 *     - even with pinning weights, drop to -1.0, etc.
 *   - maybe simply good to break dependency wherever possible?
 */

#ifndef NEGATE_NETWORK_H
#define NEGATE_NETWORK_H

#include <fstream>
#include <vector>

#include "abstract_network.h"

class NegateNetworkHistory;
class NegateNetwork : public AbstractNetwork {
public:
	int state_index;

	NegateNetwork(int state_index);
	NegateNetwork(NegateNetwork* original);
	NegateNetwork(std::ifstream& input_file);

	void save(std::ofstream& output_file);
};

class NegateNetworkHistory : public AbstractNetworkHistory {
public:
	NegateNetworkHistory(NegateNetwork* network);
};

#endif /* NEGATE_NETWORK_H */