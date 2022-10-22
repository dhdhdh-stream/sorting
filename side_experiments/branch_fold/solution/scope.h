#ifndef SCOPE_H
#define SCOPE_H

#include <vector>

#include "network.h"
#include "node.h"

const int SCOPE_TYPE_BASE = 0;
const int SCOPE_TYPE_SCOPE = 1;

class AbstractScope {
public:
	int type;

	virtual ~AbstractScope() {};
};

class BaseScope : public AbstractScope {
public:
	int num_outputs;

	BaseScope(int num_outputs);
	~BaseScope();
};

class Scope : public AbstractScope {
public:
	std::vector<AbstractScope*> actions;

	int num_inputs;
	int num_outputs;

	std::vector<std::vector<int>> input_sizes;
	std::vector<std::vector<Network*>> input_networks;

	std::vector<bool> is_simple_append;
	std::vector<int> new_layer_sizes;
	std::vector<Network*> obs_networks;	// i.e., compression_network

	std::vector<std::vector<int>> outer_input_indexes;

	std::vector<Network*> score_networks;

	std::vector<std::vector<int>> inner_input_sizes;
	std::vector<std::vector<Network*>> inner_input_networks;

	std::vector<int> inner_compress_num_layers;
	std::vector<int> inner_compress_new_sizes;
	std::vector<Network*> inner_compression_networks;	// add 2 compress 1, or add new state + compress for 1st
	std::vector<std::vector<int>> inner_compressed_scope_sizes;

	std::vector<int> end_compressed_scope_sizes;	// earliest to latest

	std::vector<double> outputs;

	std::vector<Scope*> zero_scopes;

	Scope(std::vector<AbstractScope*> actions,
		  int num_inputs,
		  int num_outputs,
		  std::vector<std::vector<int>> input_sizes,
		  std::vector<std::vector<Network*>> input_networks,
		  std::vector<bool> is_simple_append,
		  std::vector<int> new_layer_sizes,
		  std::vector<Network*> obs_networks,
		  std::vector<std::vector<int>> outer_input_indexes,
		  std::vector<Network*> score_networks,
		  std::vector<std::vector<int>> inner_input_sizes,
		  std::vector<std::vector<Network*>> inner_input_networks,
		  std::vector<int> inner_compress_num_layers,
		  std::vector<int> inner_compress_new_sizes,
		  std::vector<Network*> inner_compression_networks,
		  std::vector<std::vector<int>> inner_compressed_scope_sizes,
		  std::vector<int> end_compressed_scope_sizes);
	Scope(Scope* original);
	~Scope();
	void activate(std::vector<std::vector<double>>& flat_vals,
				  std::vector<double> inputs,
				  double& predicted_score);
	void backprop(std::vector<double> input_errors,
				  std::vector<double>& output_errors,
				  double& predicted_score,
				  double target_val);
	void add_to_dictionary(std::vector<Scope*>& scope_dictionary);

	void setup_zero_scopes();
	void zero_train_activate(std::vector<std::vector<double>>& flat_vals,
							 std::vector<double> inputs,
							 double& predicted_score);
	void zero_train_backprop(Scope* original);
	void zero_train_backprop(std::vector<double> input_errors,
							 std::vector<double>& output_errors,
							 Scope* original);
	void add_to_zero_train_dictionary(std::vector<Scope*>& scope_dictionary);
};

Scope* construct_scope(std::vector<Node*> nodes);

#endif /* SCOPE_H */