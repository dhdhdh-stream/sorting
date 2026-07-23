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
	int state;

	double state_input;

	NegateNetwork(int state);
	NegateNetwork(NegateNetwork* original);
	NegateNetwork(std::ifstream& input_file);

	void activate(std::vector<double>& state_vals);

	void save(NegateNetworkHistory* history);
	void load(NegateNetworkHistory* history);

	void backprop_through(std::vector<double>& state_errors);

	void save(std::ofstream& output_file);
};

class NegateNetworkHistory : public AbstractNetworkHistory {
public:
	double state_input_history;

	NegateNetworkHistory(NegateNetwork* network);
};

#endif /* NEGATE_NETWORK_H */