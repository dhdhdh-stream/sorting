#ifndef DECISION_NETWORK_H
#define DECISION_NETWORK_H

#include <vector>

#include "layer.h"

class DecisionNetwork {
public:
	Layer* obs_input;
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	DecisionNetwork(int num_obs,
					int num_state);
	DecisionNetwork(std::ifstream& input_file);
	~DecisionNetwork();

	void activate(std::vector<double>& obs_vals,
				  std::vector<double>& state_vals);
	void backprop(std::vector<double>& obs_vals,
				  std::vector<double>& state_vals,
				  double eval);

	void save(std::ofstream& output_file);
};

#endif /* DECISION_NETWORK_H */