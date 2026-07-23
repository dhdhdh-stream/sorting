#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

#include <vector>

#include <Eigen/Dense>

#include "abstract_network.h"
#include "layer.h"

class ScoreNetworkHistory;
class ScoreNetwork : public AbstractNetwork {
public:
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	ScoreNetwork(int num_states);
	ScoreNetwork(ScoreNetwork* original);
	ScoreNetwork(std::ifstream& input_file);
	~ScoreNetwork();

	void activate(std::vector<double>& state_vals);

	void save(ScoreNetworkHistory* history);
	void load(ScoreNetworkHistory* history);

	void init_backprop(double target_val);
	void init_update(double& hidden_1_average_max_update,
					 double& hidden_2_average_max_update,
					 double& output_average_max_update);

	void backprop(double target_val,
				  std::vector<double>& state_errors);

	void get_max_update(double& max_update_size);
	void update_weights(double learning_rate);

	void clear_update_weights();

	void add_states(int new_num_states);

	void save(std::ofstream& output_file);
};

class ScoreNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<double> state_input_history;
	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;
	double output_history;

	ScoreNetworkHistory(ScoreNetwork* network);
};

#endif /* SCORE_NETWORK_H */