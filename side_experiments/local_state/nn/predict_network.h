#ifndef PREDICT_NETWORK_H
#define PREDICT_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "layer.h"

class PredictNetwork {
public:
	Layer* state_input;

	Layer* dist_hidden_1;
	Layer* dist_hidden_2;

	Layer* mean_output;
	Layer* deviation_output;

	Layer* seed_input;

	std::vector<Layer*> noise_hidden_1s;
	std::vector<Layer*> noise_hidden_2s;

	std::vector<Layer*> noise_outputs;

	int epoch_iter;

	PredictNetwork(int num_states);
	PredictNetwork(PredictNetwork* original);
	PredictNetwork(std::ifstream& input_file);
	~PredictNetwork();

	void activate(Eigen::VectorXf& state_vals);

	void backprop(Eigen::VectorXf& starting_state_vals,
				  Eigen::VectorXf& state_diff_vals);

	void clear_momentum();

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

#endif /* PREDICT_NETWORK_H */