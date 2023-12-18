#ifndef FORGET_NETWORK_H
#define FORGET_NETWORK_H

#include <fstream>
#include <vector>

#include "layer.h"

class ForgetNetworkHistory;
class ForgetNetwork {
public:
	Layer* obs_input;
	Layer* index_input;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	ForgetNetwork();
	ForgetNetwork(std::ifstream& input_file);
	~ForgetNetwork();

	void activate(double obs_val,
				  double index_val,
				  ForgetNetworkHistory* history);
	void backprop(double error,
				  ForgetNetworkHistory* history);

	void activate(double obs_val,
				  double index_val);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class ForgetNetworkHistory {
public:
	ForgetNetwork* network;

	double obs_input_history;
	double index_input_history;
	std::vector<double> hidden_history;

	ForgetNetworkHistory(ForgetNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* FORGET_NETWORK_H */