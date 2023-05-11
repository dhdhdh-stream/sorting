#ifndef NETWORK_H
#define NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "layer.h"

class NetworkHistory;
class Network {
public:
	int input_size;
	Layer* input;
	int hidden_size;
	Layer* hidden;
	int output_size;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	Network(int input_size,
			int hidden_size,
			int output_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& vals);
	void activate(std::vector<double>& vals,
				  NetworkHistory* history);
	void backprop(std::vector<double>& errors,
				  double target_max_update);
	void backprop(std::vector<double>& errors,
				  double target_max_update,
				  NetworkHistory* history);

	void backprop_errors_with_no_weight_change(std::vector<double>& errors);
	void backprop_errors_with_no_weight_change(std::vector<double>& errors,
											   NetworkHistory* history);
	void backprop_weights_with_no_error_signal(std::vector<double>& errors,
											   double target_max_update);
	void backprop_weights_with_no_error_signal(std::vector<double>& errors,
											   double target_max_update,
											   NetworkHistory* history);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class NetworkHistory {
public:
	Network* network;

	std::vector<double> input_history;
	std::vector<double> hidden_history;

	NetworkHistory(Network* network);
	void save_weights();
	void reset_weights();
};

#endif /* NETWORK_H */