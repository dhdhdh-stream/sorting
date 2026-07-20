#ifndef NEGATE_NETWORK_H
#define NEGATE_NETWORK_H

#include <fstream>
#include <vector>

#include "abstract_network.h"

class NegateNetworkHistory;
class NegateNetwork : public AbstractNetwork {
public:
	std::vector<int> init_states;

	NegateNetwork(std::vector<int>& init_states);
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