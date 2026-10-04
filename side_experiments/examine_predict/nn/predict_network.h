#ifndef PREDICT_NETWORK_H
#define PREDICT_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "layer.h"

class PredictNetworkHistory;
class PredictNetwork {
public:
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int num_instances;
	int last_update_iter;
	int epoch_iter;

	PredictNetwork(int num_states);
	PredictNetwork(PredictNetwork* original);
	PredictNetwork(std::ifstream& input_file);
	~PredictNetwork();

	void activate(Eigen::VectorXf& state_vals);

	void save(PredictNetworkHistory* history);
	void load(PredictNetworkHistory* history);

	void backprop(Eigen::VectorXf& state_errors);

	void update();

	void clear_momentum();

	void save(std::ofstream& output_file);
};

class PredictNetworkHistory {
public:
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;
};

#endif /* PREDICT_NETWORK_H */