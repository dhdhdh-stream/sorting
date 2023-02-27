#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

class StateNetwork {
public:
	int obs_size;	// can be 0
	Layer* obs_input;

	int input_state_size;
	Layer* input_state_input;

	int local_state_size;
	Layer* local_state_input;

	int new_state_size;
	Layer* new_state_input;

	int hidden_size;
	Layer* hidden;

	Layer* output;	// size always 1

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	StateNetwork(int obs_size,
				 int input_state_size,
				 int local_state_size,
				 int new_state_size,
				 int hidden_size);
	StateNetwork(StateNetwork* original);
	StateNetwork(std::ifstream& input_file);
	~StateNetwork();

	void activate(double obs_val,
				  std::vector<double>& input_state_vals,
				  std::vector<double>& local_state_vals);
	void activate(double obs_val,
				  std::vector<double>& input_state_vals,
				  std::vector<double>& local_state_vals,
				  StateNetworkHistory* history);
	void activate(std::vector<double>& input_state_vals,
				  std::vector<double>& local_state_vals);
	void activate(std::vector<double>& input_state_vals,
				  std::vector<double>& local_state_vals,
				  StateNetworkHistory* history);
	void backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<double>& input_state_errors,
		std::vector<double>& local_state_errors);
	void backprop_errors_with_no_weight_change(
		double output_error,
		std::vector<double>& input_state_errors,
		std::vector<double>& local_state_errors,
		StateNetworkHistory* history);
	void backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update);
	void backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update,
		StateNetworkHistory* history);

	void new_outer_activate(double obs_val,
							std::vector<double>& input_state_vals,
							std::vector<double>& local_state_vals,
							double& new_state_val);
	void new_outer_activate(double obs_val,
							std::vector<double>& input_state_vals,
							std::vector<double>& local_state_vals,
							double& new_state_val,
							StateNetworkHistory* history);
	void new_outer_backprop(double output_error,
							double& new_state_error);
	void new_outer_backprop(double output_error,
							double& new_state_error,
							StateNetworkHistory* history);

	void new_sequence_activate(double obs_val,
							   std::vector<double>& input_state_vals,
							   std::vector<double>& local_state_vals,
							   std::vector<double>& new_state_vals);
	void new_sequence_activate(double obs_val,
							   std::vector<double>& input_state_vals,
							   std::vector<double>& local_state_vals,
							   std::vector<double>& new_state_vals,
							   StateNetworkHistory* history);
	void new_sequence_activate(std::vector<double>& input_state_vals,
							   std::vector<double>& local_state_vals,
							   std::vector<double>& new_state_vals);
	void new_sequence_activate(std::vector<double>& input_state_vals,
							   std::vector<double>& local_state_vals,
							   std::vector<double>& new_state_vals,
							   ScoreNetworkHistory* history);
	void new_sequence_backprop(double output_error,
							   std::vector<double>& input_state_errors,
							   std::vector<double>& local_state_errors,
							   std::vector<double>& new_state_errors);
	void new_sequence_backprop(double output_error,
							   std::vector<double>& input_state_errors,
							   std::vector<double>& local_state_errors,
							   std::vector<double>& new_state_errors,
							   StateNetworkHistory* history);

	// when permanently adding to Scope, update as might have been incremented
	void update_state_sizes(int new_input_state_size,	// before new update
							int new_local_state_size);

	void new_to_local();
	void new_to_input();

	// when readjusting, ordering is:
	// - old input
	// - old local
	// - new local
	void split_new(int split_index);
	void remove_input(int index);
	void remove_local(int index);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class StateNetworkHistory {
public:
	StateNetwork* network;

	double obs_input_history;
	std::vector<double> input_state_input_history;
	std::vector<double> local_state_input_history;
	std::vector<double> new_state_input_history;
	std::vector<double> hidden_history;

	StateNetworkHistory(StateNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* STATE_NETWORK_H */