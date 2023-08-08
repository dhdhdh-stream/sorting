#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

#include <fstream>
#include <vector>

class Layer;

class ScoreNetworkHistory;
class ScoreNetwork {
public:
	int state_size;
	Layer* state_input;

	int new_state_size;
	Layer* new_state_input;

	std::vector<std::vector<double>> lasso_weights;

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

	void new_activate(std::vector<double>& state_vals,
					  std::vector<double>& new_state_vals);
	void new_activate(std::vector<double>& state_vals,
					  std::vector<double>& new_state_vals,
					  ScoreNetworkHistory* history);
	void new_backprop(double output_error,
					  std::vector<double>& new_state_errors,
					  double target_max_update);
	void new_backprop(double output_error,
					  std::vector<double>& new_state_errors,
					  double target_max_update,
					  std::vector<double>& state_vals_snapshot,
					  std::vector<double>& new_state_vals_snapshot,
					  ScoreNetworkHistory* history);
	void new_lasso_backprop(double output_error,
							std::vector<double>& new_state_errors,
							double target_max_update);
	void new_lasso_backprop(double output_error,
							std::vector<double>& new_state_errors,
							double target_max_update,
							std::vector<double>& state_vals_snapshot,
							std::vector<double>& new_state_vals_snapshot,
							ScoreNetworkHistory* history);
	/**
	 * - TODO: always lasso later on, and remove state/classes that are no longer needed
	 */

	void update_lasso_weights(int new_furthest_distance);

	void finalize_new_state(int new_state_index);
	void cleanup_new_state();

	void add_state();

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