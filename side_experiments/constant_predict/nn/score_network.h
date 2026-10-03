#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "layer.h"

class ScoreNetwork {
public:
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int num_instances;
	int last_update_iter;
	int epoch_iter;

	ScoreNetwork(int num_states);
	ScoreNetwork(ScoreNetwork* original);
	ScoreNetwork(std::ifstream& input_file);
	~ScoreNetwork();

	void activate(Eigen::VectorXf& state_vals);

	void backprop(double target_val,
				  Eigen::VectorXf& state_errors);

	void update();

	void clear_momentum();

	void save(std::ofstream& output_file);
};

#endif /* SCORE_NETWORK_H */