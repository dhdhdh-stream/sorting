#ifndef CELL_NETWORK_H
#define CELL_NETWORK_H

#include <vector>

#include "layer.h"

class CellNetworkHistory;
class CellNetwork {
public:
	Layer* obs_input;
	Layer* action_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	CellNetwork(int num_obs,
				int num_actions);
	CellNetwork(std::ifstream& input_file);
	~CellNetwork();

	void activate(std::vector<double>& obs_vals,
				  int action);
	void activate(std::vector<double>& obs_vals,
				  int action,
				  CellNetworkHistory* history);
	void backprop(double error,
				  CellNetworkHistory* history);
	void update_weights();

	void save(std::ofstream& output_file);
};

class CellNetworkHistory {
public:
	std::vector<double> obs_input_histories;
	std::vector<double> action_input_histories;
	std::vector<double> hidden_1_histories;
	std::vector<double> hidden_2_histories;
	std::vector<double> output_histories;
};

#endif /* CELL_NETWORK_H */