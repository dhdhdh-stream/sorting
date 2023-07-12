#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

class ScoreNetwork {
public:
	int state_size;
	Layer* state_input;

	int new_state_size;
	Layer* new_state_input;

	int hidden_size;
	Layer* hidden;

	Layer* output;	// size always 1

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	ScoreNetwork(int state_size,
				 int new_state_size,
				 int hidden_size);
	ScoreNetwork(ScoreNetwork* original);
	ScoreNetwork(std::ifstream& input_file);
	~ScoreNetwork();

	void activate(std::vector<double>& state_vals);
	void activate(std::vector<double>& state_vals,
				  ScoreNetworkHistory* history);
	void backprop_errors_with_no_weight_change(double output_error,
											   std::vector<double>& state_errors);
	void backprop_errors_with_no_weight_change(double output_error,
											   std::vector<double>& state_errors,
											   std::vector<double>& state_vals_snapshot,
											   ScoreNetworkHistory* history);
	void backprop_weights_with_no_error_signal(double output_error,
											   double target_max_update);
	void backprop_weights_with_no_error_signal(double output_error,
											   double target_max_update,
											   std::vector<double>& state_vals_snapshot,
											   ScoreNetworkHistory* history);

	void backprop(double output_error,
				  std::vector<double>& new_state_errors,
				  double target_max_update);
	void backprop(double output_error,
				  std::vector<double>& new_state_errors,
				  double target_max_update,
				  std::vector<double>& new_state_vals_snapshot,
				  ScoreNetworkHistory* history);

	void new_activate(std::vector<double>& state_vals,
					  std::vector<double>& new_state_vals);
	void new_activate(std::vector<double>& state_vals,
					  std::vector<double>& new_state_vals,
					  ScoreNetworkHistory* history);
	/**
	 * - don't backprop errors into original state
	 *   - let original score networks have full impact
	 */
	void new_backprop(double output_error,
					  std::vector<double>& new_state_errors,
					  double target_max_update);
	void new_backprop(double output_error,
					  std::vector<double>& new_state_errors,
					  double target_max_update,
					  std::vector<double>& state_vals_snapshot,
					  std::vector<double>& new_state_vals_snapshot,
					  ScoreNetworkHistory* history);

	void clean(int num_new_states);
	// new states at the end of total states
	void finalize_new_state(int new_total_states);

	void add_new_state(int new_state_size);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class ScoreNetworkHistory {
public:
	ScoreNetwork* network;

	std::vector<double> hidden_history;

	ScoreNetworkHistory(ScoreNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* SCORE_NETWORK_H */