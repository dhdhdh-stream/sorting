#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "layer.h"

class ScoreNetworkHistory;
class ScoreNetwork {
public:
	std::vector<int> init_states;

	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int num_instances;
	int last_update_iter;
	int epoch_iter;

	ScoreNetwork(std::vector<int> init_states);
	ScoreNetwork(ScoreNetwork* original);
	ScoreNetwork(std::ifstream& input_file);
	~ScoreNetwork();

	void activate(Eigen::VectorXf& state_vals);

	void init_backprop(double target_val);

	void init_activate(std::vector<double>& new_state_vals);
	void init_backprop(double target_val,
					   std::vector<double>& new_state_errors);

	void init_update();

	void save(ScoreNetworkHistory* history);
	void load(ScoreNetworkHistory* history);

	void backprop(double target_val,
				  Eigen::VectorXf& state_errors);

	void update();

	void clear_momentum();

	void save(std::ofstream& output_file);
};

class ScoreNetworkHistory {
public:
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;
	double output_history;
};

#endif /* SCORE_NETWORK_H */