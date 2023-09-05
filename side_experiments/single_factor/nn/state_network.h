#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

#include <fstream>
#include <vector>

const int STATE_NETWORK_HIDDEN_SIZE = 10;

class Layer;

class StateNetworkHistory;
class StateNetwork {
public:
	Layer* obs_input;
	Layer* state_input;

	Layer* hidden;

	Layer* output;

	// update outside
	double ending_state_mean;
	double ending_state_variance;
	double ending_state_standard_deviation;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	StateNetwork();
	StateNetwork(StateNetwork* original);
	StateNetwork(std::ifstream& input_file);
	~StateNetwork();

	void activate(double obs_val,
				  double& state_val);
	void activate(double obs_val,
				  double& state_val,
				  StateNetworkHistory* history);
	void backprop(double& state_error,
				  double target_max_update);
	void backprop(double& state_error,
				  double target_max_update,
				  double obs_snapshot,
				  StateNetworkHistory* history);
	void backprop_errors_with_no_weight_change(
		double& state_error);
	void backprop_errors_with_no_weight_change(
		double& state_error,
		double obs_snapshot,
		StateNetworkHistory* history);
	void backprop_errors_with_no_weight_change(
		double& obs_error,
		double& state_error);
	void backprop_errors_with_no_weight_change(
		double& obs_error,
		double& state_error,
		double obs_snapshot,
		StateNetworkHistory* history);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class StateNetworkHistory {
public:
	StateNetwork* network;

	double state_history;
	std::vector<double> hidden_history;

	StateNetworkHistory(StateNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* STATE_NETWORK_H */