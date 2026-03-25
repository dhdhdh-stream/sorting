#ifndef SIGNAL_NODE_H
#define SIGNAL_NODE_H

#include <fstream>
#include <vector>

#include "signal_input.h"

class Layer;
class Solution;

class SignalNode {
public:
	std::vector<SignalInput> inputs;
	std::vector<double> input_averages;
	std::vector<double> input_standard_deviations;

	Layer* input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* hidden_3;
	Layer* output;

	SignalNode(std::vector<SignalInput>& inputs,
			   std::vector<double>& input_averages,
			   std::vector<double>& input_standard_deviations);
	SignalNode(SignalNode* original,
			   Solution* parent_solution);
	SignalNode(std::ifstream& input_file,
			   Solution* parent_solution);
	~SignalNode();

	void activate(std::vector<double>& input_vals,
				  std::vector<bool>& input_is_on);
	void backprop();

	void get_max_update(double& max_update);
	void update_weights(double learning_rate);

	void remove_input(int index);

	void save(std::ofstream& output_file);
};

#endif /* SIGNAL_NODE_H */