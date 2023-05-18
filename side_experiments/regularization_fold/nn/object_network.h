#ifndef OBJECT_NETWORK_H
#define OBJECT_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "layer.h"

class ObjectNetworkHistory;
class ObjectNetwork {
public:
	int obs_size;	// can be 0
	Layer* obs_input;

	std::vector<int> dependency_indexes;
	Layer* dependency_input;

	int hidden_size;
	Layer* hidden;

	Layer* output;	// size always 1

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	ObjectNetwork(int obs_size,
				  int dependency_size,
				  int hidden_size);
	ObjectNetwork(ObjectNetwork* original);
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

class ObjectNetworkHistory {
public:
	ObjectNetwork* network;

	double obs_input_history;
	std::vector<double> state_input_history;
	std::vector<double> hidden_history;

	ObjectNetworkHistory(ObjectNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* OBJECT_NETWORK_H */