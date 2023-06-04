#ifndef NETWORK_H
#define NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "layer.h"

class Network {
public:
	int obs_size;	// can be 0
	Layer* obs_input;
	// lasso to determine if network needed

	int state_size;
	Layer* state_input;
	// lasso to best effort remove and simplify network

	int new_state_size;
	Layer* new_state_input;

	int hidden_size;
	Layer* hidden;

	Layer* output;	// size always 1

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	Network(int obs_size,
			int state_size,
			int new_state_size,
			int hidden_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(double obs_val,
				  std::vector<double>& state_vals);
	void activate(std::vector<double>& state_vals);
	void backprop(double target_max_update);
	void lasso_backprop(double lambda,
						double target_max_update);

	void backprop_errors_with_no_weight_change();
	void backprop_weights_with_no_error_signal(double target_max_update);

	void new_activate(double obs_val,
					  std::vector<double>& state_vals,
					  std::vector<double>& new_state_vals);
	void new_activate(std::vector<double>& state_vals,
					  std::vector<double>& new_state_vals);

	// void add_state();
	// void calc_state_impact(int index);
	// void remove_state(int index);

	void save(std::ofstream& output_file);

private:
	void construct();
};

#endif /* NETWORK_H */