#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

#include <fstream>
#include <vector>

class Layer;

class StateNetworkHistory;
class StateNetwork {
public:
	int obs_size;
	Layer* obs_input;
	double obs_scale;

	Layer* state_input;

	int hidden_size;
	Layer* hidden;

	Layer* output;

	double obs_weight;
	double final_val;
	double final_error;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	StateNetwork(int obs_size,
				 int hidden_size);
	StateNetwork(StateNetwork* original);
	StateNetwork(std::ifstream& input_file);
	~StateNetwork();

	void activate();
	void activate(StateNetworkHistory* history);
	void backprop(double target_max_update);
	void backprop(double target_max_update,
				  std::vector<double>& obs_history,
				  StateNetworkHistory* history);
	void lasso_backprop(double target_max_update);
	void lasso_backprop(double target_max_update,
						std::vector<double>& obs_history,
						StateNetworkHistory* history);
	void backprop_errors_with_no_weight_change();
	void backprop_errors_with_no_weight_change(
		std::vector<double>& obs_history,
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