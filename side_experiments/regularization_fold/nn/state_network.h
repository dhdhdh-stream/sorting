#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

class StateNetwork {
public:
	Layer* obs_input;	// size always 1 for sorting

	std::vector<int> state_indexes;
	Layer* state_input;

	std::vector<int> new_state_indexes;
	Layer* new_state_input;

	int new_input_size;	// 0 or 1
	Layer* new_input_input;

	/**
	 * - scale based on further context seen in
	 *   - update whenever scope seen in further layer
	 */
	std::vector<std::vector<double>> lasso_weights;

	int hidden_size;
	Layer* hidden;

	Layer* output;	// size always 1

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	StateNetwork(int state_size,
				 int new_state_size,
				 int new_input_size,
				 int hidden_size);
	StateNetwork(StateNetwork* original);
	StateNetwork(std::ifstream& input_file);
	~StateNetwork();

	void activate(double obs_val,
				  std::vector<double>& state_vals);
	void activate(double obs_val,
				  std::vector<double>& state_vals,
				  StateNetworkHistory* history);
	void backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<double>& state_errors);
	void backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<double>& state_errors,
		double obs_snapshot,
		std::vector<double>& state_vals_snapshot,
		StateNetworkHistory* history);

	void new_activate(double obs_val,
					  std::vector<double>& state_vals,
					  std::vector<double>& new_state_vals);
	void new_activate(double obs_val,
					  std::vector<double>& state_vals,
					  std::vector<double>& new_state_vals,
					  StateNetworkHistory* history);
	void new_backprop(double output_error,
					  std::vector<double>& new_state_errors,
					  double target_max_update);
	void new_backprop(double output_error,
					  std::vector<double>& new_state_errors,
					  double target_max_update,
					  double obs_snapshot,
					  std::vector<double>& state_vals_snapshot,
					  std::vector<double>& new_state_vals_snapshot,
					  StateNetworkHistory* history);
	void new_lasso_backprop(double output_error,
							std::vector<double>& new_state_errors,
							double target_max_update);
	void new_lasso_backprop(double output_error,
							std::vector<double>& new_state_errors,
							double target_max_update,
							double obs_snapshot,
							std::vector<double>& state_vals_snapshot,
							std::vector<double>& new_state_vals_snapshot,
							StateNetworkHistory* history);

	void new_activate(double obs_val,
					  std::vector<double>& state_vals,
					  std::vector<double>& new_state_vals,
					  double new_input_val);
	void new_activate(double obs_val,
					  std::vector<double>& state_vals,
					  std::vector<double>& new_state_vals,
					  double new_input_val,
					  StateNetworkHistory* history);
	void new_backprop(double output_error,
					  std::vector<double>& new_state_errors,
					  double& new_input_error,
					  double target_max_update);
	void new_backprop(double output_error,
					  std::vector<double>& new_state_errors,
					  double& new_input_error,
					  double target_max_update,
					  double obs_snapshot,
					  std::vector<double>& state_vals_snapshot,
					  std::vector<double>& new_state_vals_snapshot,
					  double new_input_snapshot,
					  StateNetworkHistory* history);
	void new_lasso_backprop(double output_error,
							std::vector<double>& new_state_errors,
							double& new_input_error,
							double target_max_update);
	void new_lasso_backprop(double output_error,
							std::vector<double>& new_state_errors,
							double& new_input_error,
							double target_max_update,
							double obs_snapshot,
							std::vector<double>& state_vals_snapshot,
							std::vector<double>& new_state_vals_snapshot,
							double new_input_snapshot,
							StateNetworkHistory* history);

	void update_lasso_weights(int new_furthest_distance);

	void clean(int num_new_states);
	void finalize_new_state(int layer_num_new_states,
							int new_total_states);
	void finalize_new_input(int new_index);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class StateNetworkHistory {
public:
	StateNetworkHistory* history;

	std::vector<double> hidden_history;

	StateNetworkHistory(StateNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* STATE_NETWORK_H */