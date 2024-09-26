#ifndef EVAL_NETWORK_H
#define EVAL_NETWORK_H

#include <vector>

#include "layer.h"

class EvalNetworkHistory;
class EvalNetwork {
public:
	Layer* obs_input;
	Layer* action_input;
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	EvalNetwork(int num_obs,
				int num_actions,
				int num_state);
	EvalNetwork(std::ifstream& input_file);
	~EvalNetwork();

	void activate(std::vector<double>& obs_vals,
				  int action,
				  std::vector<double>& state_vals,
				  EvalNetworkHistory* history,
				  double& output);
	void activate(std::vector<double>& obs_vals,
				  int action,
				  std::vector<double>& state_vals,
				  double& output);
	void backprop(double error,
				  std::vector<double>& state_errors,
				  EvalNetworkHistory* history);

	void save(std::ofstream& output_file);
};

class EvalNetworkHistory {
public:
	std::vector<double> obs_input_history;
	std::vector<double> action_input_history;
	std::vector<double> state_input_history;

	std::vector<double> hidden_1_history;
	std::vector<double> hidden_2_history;
};

#endif /* EVAL_NETWORK_H */