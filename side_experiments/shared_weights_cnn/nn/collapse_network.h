#ifndef COLLAPSE_NETWORK_H
#define COLLAPSE_NETWORK_H

#include <fstream>
#include <vector>

#include "layer.h"

const int FLAT = 0;
const int COLLAPSE = 1;
const int FOLD = 2;

class CollapseNetwork {
public:
	int time_size;
	int max_iters;
	int global_size;

	int state;
	int index;

	int collapse_state_size;
	int fold_state_size;

	Layer* global_input;
	
	Layer* collapse_input;
	std::vector<std::vector<double>> collapse_input_historys;
	Layer* collapse_layer;
	std::vector<std::vector<double>> collapse_layer_historys;
	Layer* collapse_output;
	// don't need collapse_output_historys for now

	Layer* fold_collapse_input;
	std::vector<std::vector<double>> fold_collapse_input_historys;
	Layer* fold_state_input;
	std::vector<std::vector<double>> fold_state_input_historys;
	Layer* fold_layer;
	std::vector<std::vector<double>> fold_layer_historys;
	Layer* fold_output;
	// don't need fold_output_historys for now

	std::vector<Layer*> process_inputs;
	Layer* process;
	Layer* output;

	CollapseNetwork(int time_size,
					int max_iters,
					int global_size);
	CollapseNetwork(CollapseNetwork* original);
	~CollapseNetwork();

	void activate(int num_iterations,
				  std::vector<std::vector<double>>& time_vals,
				  std::vector<double>& global_vals);
	void backprop(std::vector<double>& errors);
	void calc_max_update(double& max_update,
						 double learning_rate,
						 double momentum);
	void update_weights(double factor,
						double learning_rate,
						double momentum);

	void next_step();

private:
	void push_collapse_history();
	void pop_collapse_history();

	void push_fold_history();
	void pop_fold_history();
};

#endif /* COLLAPSE_NETWORK_H */