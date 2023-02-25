#ifndef SCOPE_NETWORK_H
#define SCOPE_NETWORK_H

class ScopeNetwork {
public:
	// Note: original_input_state_size may be less than input_state_vals.size(), etc.
	int original_input_state_size;
	int original_local_state_size;
	int new_input_state_size;
	int new_local_state_size;
	Layer* input_state_input;
	Layer* local_state_input;

	int hidden_size;
	Layer* hidden;

	Layer* output;	// size always 1

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	ScopeNetwork(int original_input_state_size,
				 int original_local_state_size,
				 int new_input_state_size,
				 int new_local_state_size,
				 int hidden_size);
	ScopeNetwork(ScopeNetwork* original);
	ScopeNetwork(std::ifstream& input_file);
	~ScopeNetwork();

	void activate(std::vector<double>& input_state_vals,
				  std::vector<double>& local_state_vals);
	void activate(std::vector<double>& input_state_vals,
				  std::vector<double>& local_state_vals,
				  ScopeNetworkHistory* history);
	void backprop(double output_error,
				  std::vector<double>& input_state_errors,
				  std::vector<double>& local_state_errors);
	void backprop(double output_error,
				  std::vector<double>& input_state_errors,
				  std::vector<double>& local_state_errors,
				  ScopeNetworkHistory* history);

	void add_new_input_state();	// for updating new score networks
	void new_input_activate(std::vector<double>& input_state_vals,
							std::vector<double>& new_input_state_vals,
							std::vector<double>& local_state_vals);
	void new_input_activate(std::vector<double>& input_state_vals,
							std::vector<double>& new_input_state_vals,
							std::vector<double>& local_state_vals,
							ScopeNetworkHistory* history);
	// only backprop new_input_state
	void new_input_backprop(double output_error,
							std::vector<double>& new_input_state_errors);
	void new_input_backprop(double output_error,
							std::vector<double>& new_input_state_errors,
							ScopeNetworkHistory* history);

	void add_new_local_state();
	void new_local_activate(std::vector<double>& input_state_vals,
							std::vector<double>& local_state_vals,
							std::vector<double>& new_local_state_vals);
	void new_local_activate(std::vector<double>& input_state_vals,
							std::vector<double>& local_state_vals,
							std::vector<double>& new_local_state_vals,
							ScopeNetworkHistory* history);
	void new_local_backprop(double output_error,
							std::vector<double>& new_local_state_errors);
	void new_local_backprop(double output_error,
							std::vector<double>& new_local_state_errors,
							ScopeNetworkHistory* history);

	// when permanently adding to Scope, update as might have been incremented
	void update_state_sizes(int input_state_size,	// before new update
							int local_state_size);
	// for existing score networks
	void add_input_state(int num_new_input_state);
	void add_local_state(int num_new_local_state);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class ScopeNetworkHistory {
public:
	ScopeNetwork* network;

	std::vector<double> input_state_input_history;
	std::vector<double> local_state_input_history;
	std::vector<double> hidden_history;

	ScopeNetworkHistory(ScopeNetwork* network);
	void save_weights();
	void reset_weights();
};

#endif /* SCOPE_NETWORK_H */