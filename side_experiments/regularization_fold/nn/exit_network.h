#ifndef EXIT_NETWORK_H
#define EXIT_NETWORK_H

class ExitNetwork {
public:
	std::vector<int> context_indexes;
	std::vector<int> state_indexes;
	Layer* state_input;

	std::vector<int> new_state_indexes;
	Layer* new_state_input;

	std::vector<std::vector<double>> lasso_weights;

	int hidden_size;
	Layer* hidden;

	Layer* output;	// size always 1

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	ExitNetwork(std::vector<std::vector<int>>& context_sizes,
				int new_state_size,
				int hidden_size);
	ExitNetwork(ExitNetwork* original);
	ExitNetwork(std::ifstream& input_file);
	~ExitNetwork();

	void activate(std::vector<std::vector<double>>& state_vals);
	void activate(std::vector<std::vector<double>>& state_vals,
				  ExitNetworkHistory* history);
	void backprop_errors_with_no_weight_change(double output_error,
											   std::vector<std::vector<double>*>& state_errors);
	void backprop_errors_with_no_weight_change(double output_error,
											   std::vector<std::vector<double>*>& state_errors,
											   std::vector<std::vector<double>>& state_vals_snapshot,
											   ExitNetworkHistory* history);

	void new_activate(std::vector<std::vector<double>>& state_vals,
					  std::vector<double>& new_state_vals);
	void new_activate(std::vector<std::vector<double>>& state_vals,
					  std::vector<double>& new_state_vals,
					  ExitNetworkHistory* history);
	void new_backprop(double output_error,
					  std::vector<double>& new_state_errors,
					  double target_max_update);
	void new_backprop(double output_error,
					  std::vector<double>& new_state_errors,
					  double target_max_update,
					  std::vector<std::vector<double>>& state_vals_snapshot,
					  std::vector<double>& new_state_vals_snapshot,
					  ExitNetworkHistory* history);
	void new_lasso_backprop(double output_error,
							std::vector<double>& new_state_errors,
							double target_max_update);
	void new_lasso_backprop(double output_error,
							std::vector<double>& new_state_errors,
							double target_max_update,
							std::vector<std::vector<double>>& state_vals_snapshot,
							std::vector<double>& new_state_vals_snapshot,
							ExitNetworkHistory* history);

	void clean(int num_new_states);
	/**
	 * - same state may appear multiple times in context, but use at final layer
	 *   - (remaining new state becomes start of new final layer)
	 */
	void finalize_new_state(int exit_depth);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class ExitNetworkHistory {
public:
	ExitNetwork* network;

	std::vector<double> hidden_history;

	ExitNetworkHistory(ExitNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* EXIT_NETWORK_H */