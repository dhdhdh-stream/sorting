#ifndef FOLD_NETWORK_H
#define FOLD_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "layer.h"

const int FLAT = 0;
const int FOLD = 1;
const int CLEAN = 2;

class FoldNetwork {
public:
	int time_size;
	int max_iters;
	int global_size;

	int fold_state_size;

	int state;
	int index;

	Layer* global_input;

	Layer* fold_input;
	std::vector<std::vector<double>> fold_input_historys;
	Layer* fold_state_input;
	std::vector<std::vector<double>> fold_state_input_historys;
	Layer* fold_layer;
	std::vector<std::vector<double>> fold_layer_historys;
	Layer* fold_output;
	// don't need fold_output_historys for now

	std::vector<Layer*> process_inputs;
	Layer* process;
	Layer* output;

	int epoch;
	int iter;

	std::mutex mtx;

	FoldNetwork(int time_size,
				int max_iters,
				int global_size,
				int fold_state_size);
	FoldNetwork(FoldNetwork* original);
	FoldNetwork(std::ifstream& input_file);
	~FoldNetwork();

	void train(int num_iterations,
			   std::vector<std::vector<double>>& time_vals,
			   std::vector<double>& global_vals);
	void backprop(std::vector<double>& errors);
	void increment();
	void calc_max_update(double& max_update,
						 double learning_rate,
						 double momentum);
	void update_weights(double factor,
						double learning_rate,
						double momentum);

	void next_step();

	std::vector<double> fan(int num_iterations,
							std::vector<std::vector<double>>& time_vals,
							std::vector<double>& global_vals);

	void save(std::ofstream& output_file);

private:
	void setup_layers();

	void push_fold_history();
	void pop_fold_history();
};

#endif /* FOLD_NETWORK_H */