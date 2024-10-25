#ifndef GATE_H
#define GATE_H

#include <vector>

#include "layer.h"

class GateHistory;
class Gate {
public:
	Layer* obs_input;
	Layer* action_input;
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	Gate(int num_obs,
		 int num_actions,
		 int num_states);
	Gate(std::ifstream& input_file);
	~Gate();

	void activate(std::vector<double>& obs_vals,
				  int action,
				  std::vector<double>& state_vals);
	void activate(std::vector<double>& obs_vals,
				  int action,
				  std::vector<double>& state_vals,
				  GateHistory* history);
	void backprop(double error,
				  std::vector<double>& state_errors,
				  GateHistory* history);
	void update_weights();

	void save(std::ofstream& output_file);
};

class GateHistory {
public:
	std::vector<double> obs_input_histories;
	std::vector<double> action_input_histories;
	std::vector<double> state_input_histories;
	std::vector<double> hidden_1_histories;
	std::vector<double> hidden_2_histories;
	std::vector<double> output_histories;
};

#endif /* GATE_H */