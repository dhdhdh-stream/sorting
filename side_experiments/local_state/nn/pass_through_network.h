/**
 * - add to
 */

#ifndef PASS_THROUGH_NETWORK_H
#define PASS_THROUGH_NETWORK_H

#include <fstream>

#include "abstract_network.h"

class PassThroughNetwork : public AbstractNetwork {
public:
	int front_state_index;
	int back_state_index;

	PassThroughNetwork(int front_state_index,
					   int back_state_index);
	PassThroughNetwork(PassThroughNetwork* original);
	PassThroughNetwork(std::ifstream& input_file);

	void save(std::ofstream& output_file);
};

class PassThroughNetworkHistory : public AbstractNetworkHistory {
public:
	PassThroughNetworkHistory(PassThroughNetwork* network);
};

#endif /* PASS_THROUGH_NETWORK_H */