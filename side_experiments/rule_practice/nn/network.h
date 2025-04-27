#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "layer.h"

class NetworkHistory;
class Network {
public:
	int unroll_iters;
	int num_obs;
	int num_actions;

	Layer* input;
	Layer* hidden_1;
	Layer* hidden_2;
	Layer* hidden_3;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double hidden_3_average_max_update;
	double output_average_max_update;

	Network(int unroll_iters,
			int num_obs,
			int num_actions);
	~Network();

	void activate(std::vector<std::vector<double>>& obs_vals,
				  std::vector<int>& moves);
	void activate(std::vector<std::vector<double>>& obs_vals,
				  std::vector<int>& moves,
				  NetworkHistory* history);
	void backprop(double error,
				  NetworkHistory* history);
};

class NetworkHistory {
public:
	std::vector<double> input_history;
	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;
	std::vector<double> hidden_3_history;
};

#endif /* NETWORK_H */