#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

#include <fstream>
#include <vector>

#include "layer.h"

class StateNetworkHistory;
class StateNetwork {
public:
	int obs_size;	// can be 0
	Layer* obs_input;

	int state_size;
	Layer* state_input;

	int new_inner_state_size;
	Layer* new_inner_state_input;
	int new_outer_state_size;		// also new_external_state_size
	Layer* new_outer_state_input;	// also new_external_state_input

	int hidden_size;
	Layer* hidden;

	Layer* output;	// size always 1

	std::vector<bool> state_zeroed;
	std::vector<bool> new_inner_state_zeroed;
	std::vector<bool> new_outer_state_zeroed;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	StateNetwork(int obs_size,
				 int state_size,
				 int new_inner_state_size,
				 int new_outer_state_size,
				 int hidden_size);
	StateNetwork(StateNetwork* original);
	StateNetwork(std::ifstream& input_file);
	~StateNetwork();

	void activate(double obs_val,
				  std::vector<double>& state_vals);
	void activate(double obs_val,
				  std::vector<double>& state_vals,
				  StateNetworkHistory* history);
	void activate(std::vector<double>& state_vals);
	void activate(std::vector<double>& state_vals,
				  StateNetworkHistory* history);
	void backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<double>& state_errors);
	void backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<double>& state_errors,
		StateNetworkHistory* history);
	void backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update);
	void backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update,
		StateNetworkHistory* history);

	void new_external_activate(double obs_val,
							   std::vector<double>& state_vals,
							   std::vector<double>& new_external_state_vals);
	void new_external_activate(double obs_val,
							   std::vector<double>& state_vals,
							   std::vector<double>& new_external_state_vals,
							   StateNetworkHistory* history);
	void new_external_activate(std::vector<double>& state_vals,
							   std::vector<double>& new_external_state_vals);
	void new_external_activate(std::vector<double>& state_vals,
							   std::vector<double>& new_external_state_vals,
							   StateNetworkHistory* history);
	void new_external_backprop(double output_error,
							   std::vector<double>& new_external_state_errors,
							   double target_max_update);
	void new_external_backprop(double output_error,
							   std::vector<double>& new_external_state_errors,
							   double target_max_update,
							   StateNetworkHistory* history);
	void new_external_backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<double>& new_external_state_errors);
	void new_external_backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<double>& new_external_state_errors,
		StateNetworkHistory* history);

	void new_sequence_activate(double obs_val,
							   std::vector<double>& new_inner_state_vals,
							   std::vector<double>& state_vals,
							   std::vector<double>& new_outer_state_vals);
	void new_sequence_activate(double obs_val,
							   std::vector<double>& new_inner_state_vals,
							   std::vector<double>& state_vals,
							   std::vector<double>& new_outer_state_vals,
							   StateNetworkHistory* history);
	void new_sequence_activate(std::vector<double>& new_inner_state_vals,
							   std::vector<double>& state_vals,
							   std::vector<double>& new_outer_state_vals);
	void new_sequence_activate(std::vector<double>& new_inner_state_vals,
							   std::vector<double>& state_vals,
							   std::vector<double>& new_outer_state_vals,
							   StateNetworkHistory* history);
	void new_sequence_backprop(double output_error,
							   std::vector<double>& new_inner_state_errors,
							   std::vector<double>& state_errors,
							   std::vector<double>& new_outer_state_errors,
							   double target_max_update);
	void new_sequence_backprop(double output_error,
							   std::vector<double>& new_inner_state_errors,
							   std::vector<double>& state_errors,
							   std::vector<double>& new_outer_state_errors,
							   double target_max_update,
							   StateNetworkHistory* history);
	void new_sequence_backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<double>& new_inner_state_errors,
		std::vector<double>& state_errors,
		std::vector<double>& new_outer_state_errors);
	void new_sequence_backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<double>& new_inner_state_errors,
		std::vector<double>& state_errors,
		std::vector<double>& new_outer_state_errors,
		StateNetworkHistory* history);

	void add_new_inner();
	void add_new_outer();

	void remove_new_outer();

	void zero_state(int index);

	// when permanently adding to Scope, update as might have been incremented
	void update_state_size(int new_state_size);	// before new update

	void new_external_to_state();

	void new_sequence_finalize();

	void remove_state(int index);
	// move then remove unneeded

	// for external score
	// (external state doesn't need to be updated)
	void add_state(int size);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class StateNetworkHistory {
public:
	StateNetwork* network;

	double obs_input_history;
	std::vector<double> state_input_history;
	std::vector<double> new_inner_state_input_history;
	std::vector<double> new_outer_state_input_history;
	std::vector<double> hidden_history;

	StateNetworkHistory(StateNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* STATE_NETWORK_H */