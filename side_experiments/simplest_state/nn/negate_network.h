#ifndef NEGATE_NETWORK_H
#define NEGATE_NETWORK_H

#include <fstream>
#include <vector>

#include "abstract_network.h"

class NegateNetworkHistory;
class NegateNetwork : public AbstractNetwork {
public:
	std::vector<int> init_states;

	std::vector<double> weights;

	std::vector<double> state_vals;

	std::vector<double> weight_updates;

	int epoch_iter;
	std::vector<double> average_max_updates;
	int last_update_iter;

	NegateNetwork(std::vector<int>& init_states);
	NegateNetwork(NegateNetwork* original);
	NegateNetwork(std::ifstream& input_file);

	void activate(std::vector<double>& state_vals);

	void save(NegateNetworkHistory* history);
	void load(NegateNetworkHistory* history);

	void backprop(std::vector<double>& state_errors);

	void update();

	void save(std::ofstream& output_file);
};

class NegateNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<double> state_vals_history;

	NegateNetworkHistory(NegateNetwork* network);
};

#endif /* NEGATE_NETWORK_H */