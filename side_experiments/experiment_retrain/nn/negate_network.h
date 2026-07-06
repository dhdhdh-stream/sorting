#ifndef NEGATE_NETWORK_H
#define NEGATE_NETWORK_H

#include <fstream>
#include <vector>

#include "abstract_network.h"

class NegateNetworkHistory;
class NegateNetwork : public AbstractNetwork {
public:
	int state;
	double weight;

	double state_input;

	double weight_update;

	NegateNetwork(int state);
	NegateNetwork(NegateNetwork* original);
	NegateNetwork(std::ifstream& input_file);

	void activate(std::vector<double>& state_vals);

	void save(NegateNetworkHistory* history);
	void load(NegateNetworkHistory* history);

	void backprop(std::vector<double>& state_errors);

	void get_max_update(double& max_update);
	void update_weights(double learning_rate);

	void save(std::ofstream& output_file);
};

class NegateNetworkHistory : public AbstractNetworkHistory {
public:
	double state_input_history;

	NegateNetworkHistory(NegateNetwork* network);
};

#endif /* NEGATE_NETWORK_H */