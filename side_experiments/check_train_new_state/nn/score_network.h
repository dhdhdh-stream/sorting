#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "layer.h"

class ScoreNetworkHistory;
class ScoreNetwork {
public:
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	ScoreNetwork(int num_states);
	~ScoreNetwork();

	void activate(std::vector<double>& state_vals);

	void init_backprop(double target_val);
	void init_update(double& hidden_1_average_max_update,
					 double& hidden_2_average_max_update,
					 double& output_average_max_update);
};

#endif /* SCORE_NETWORK_H */