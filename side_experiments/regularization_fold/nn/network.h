#ifndef NETWORK_H
#define NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "layer.h"

class NetworkHistory;
class Network {
public:
	int obs_size;	// can be 0
	Layer* obs_input;
	// lasso to determine if network needed

	int state_size;
	Layer* state_input;
	// lasso to best effort remove and simplify network

	int hidden_size;
	Layer* hidden;

	Layer* output;	// size always 1

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	Network(int obs_size,
			int state_size,
			int hidden_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate();
	void activate(NetworkHistory* history);
	void backprop(double target_max_update);
	void backprop(double target_max_update,
				  NetworkHistory* history);
	void lasso_backprop(double lambda,
						double target_max_update);
	void lasso_backprop(double lambda,
						double target_max_update,
						NetworkHistory* history);

	void backprop_errors_with_no_weight_change();
	void backprop_errors_with_no_weight_change(NetworkHistory* history);
	void backprop_weights_with_no_error_signal(double target_max_update);
	void backprop_weights_with_no_error_signal(double target_max_update,
											   NetworkHistory* history);

	void add_state();
	void calc_state_impact(int index);
	void remove_state(int index);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class NetworkHistory {
public:
	Network* network;

	double obs_input_history;
	std::vector<double> state_input_history;
	std::vector<double> hidden_history;

	NetworkHistory(Network* network);
	void save_weights();
	void reset_weights();
};

#endif /* NETWORK_H */