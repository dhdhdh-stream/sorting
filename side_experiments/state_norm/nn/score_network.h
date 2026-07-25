#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "abstract_network.h"
#include "layer.h"

class ScoreNetworkHistory;
class ScoreNetwork : public AbstractNetwork {
public:
	Eigen::VectorXf state_norms;
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int last_update_iter;
	int epoch_iter;
	double average_max_update;

	ScoreNetwork(int num_states);
	ScoreNetwork(ScoreNetwork* original);
	ScoreNetwork(std::ifstream& input_file);
	~ScoreNetwork();

	void activate(Eigen::VectorXf& state_norms,
				  Eigen::VectorXf& state_vals);

	void init_backprop(double target_val);

	void init_activate(Eigen::VectorXf& state_norms,
					   Eigen::VectorXf& state_vals,
					   int new_state_norm,
					   std::vector<double>& new_state_vals);
	void init_backprop(double target_val,
					   int new_state_norm,
					   std::vector<double>& new_state_errors);

	void init_update(double& hidden_1_average_max_update,
					 double& hidden_2_average_max_update,
					 double& output_average_max_update);

	void save(ScoreNetworkHistory* history);
	void load(ScoreNetworkHistory* history);

	void backprop(double target_val,
				  Eigen::VectorXf& state_errors);

	void update();

	void clear_update_weights();

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class ScoreNetworkHistory : public AbstractNetworkHistory {
public:
	Eigen::VectorXf state_norms_history;
	Eigen::VectorXf state_input_history;
	Eigen::VectorXf hidden_1_history;
	Eigen::VectorXf hidden_2_history;
	double output_history;

	ScoreNetworkHistory(ScoreNetwork* network);
};

#endif /* SCORE_NETWORK_H */