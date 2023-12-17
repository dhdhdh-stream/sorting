#ifndef INDEX_NETWORK_H
#define INDEX_NETWORK_H

#include <fstream>
#include <vector>

#include "layer.h"

class IndexNetworkHistory;
class IndexNetwork {
public:
	Layer* obs_input;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	IndexNetwork();
	IndexNetwork(std::ifstream& input_file);
	~IndexNetwork();

	void activate(double obs_val,
				  IndexNetworkHistory* history);
	void backprop(double error,
				  IndexNetworkHistory* history);

	void activate(double obs_val);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class IndexNetworkHistory {
public:
	IndexNetwork* network;

	double obs_input_history;
	std::vector<double> hidden_history;

	IndexNetworkHistory(IndexNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* INDEX_NETWORK_H */