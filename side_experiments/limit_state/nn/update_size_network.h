#ifndef UPDATE_SIZE_NETWORK_H
#define UPDATE_SIZE_NETWORK_H

#include <fstream>
#include <vector>

#include "layer.h"

class UpdateSizeNetworkHistory;
class UpdateSizeNetwork {
public:
	Layer* obs_input;
	Layer* index_input;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	UpdateSizeNetwork();
	UpdateSizeNetwork(std::ifstream& input_file);
	~UpdateSizeNetwork();

	void activate(double obs_val,
				  double index_val,
				  UpdateSizeNetworkHistory* history);
	void backprop(double error,
				  UpdateSizeNetworkHistory* history);

	void activate(double obs_val,
				  double index_val);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class UpdateSizeNetworkHistory {
public:
	UpdateSizeNetwork* network;

	double obs_input_history;
	double index_input_history;
	std::vector<double> hidden_history;

	UpdateSizeNetworkHistory(UpdateSizeNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* UPDATE_SIZE_NETWORK_H */