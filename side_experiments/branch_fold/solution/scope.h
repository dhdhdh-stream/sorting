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

	int num_outputs;

	std::vector<std::vector<int>> input_sizes;
	std::vector<std::vector<Network*>> input_networks;

	std::vector<int> new_layer_sizes;
	std::vector<Network*> obs_networks;	// i.e., compression_network

	std::vector<std::vector<int>> outer_input_indexes;

	// not worth to flatten inner scope
	std::vector<std::vector<int>> inner_input_layer;	// relative
	std::vector<std::vector<int>> inner_input_sizes;
	std::vector<std::vector<Network*>> inner_input_networks;

	std::vector<Network*> score_networks;

	// TODO: add compress_layers
	std::vector<int> inner_compress_new_sizes;
	std::vector<Network*> inner_compression_networks;	// compress 2, add 1, or add new state + compress

	// not worth to flatten inner scope
	int compress_num_layers;
	std::vector<int> compressed_scope_sizes;

	Scope(std::vector<AbstractScope*> actions,
		  int num_outputs,
		  std::vector<std::vector<int>> input_sizes,
		  std::vector<std::vector<Network*>> input_networks,
		  std::vector<int> new_layer_sizes,
		  std::vector<Network*> obs_networks,
		  std::vector<std::vector<int>> outer_input_indexes,
		  std::vector<std::vector<int>> inner_input_layer,
		  std::vector<std::vector<int>> inner_input_sizes,
		  std::vector<std::vector<Network*>> inner_input_networks,
		  std::vector<Network*> score_networks,
		  std::vector<int> inner_compress_new_sizes,
		  std::vector<Network*> inner_compression_networks,
		  int compress_num_layers,
		  std::vector<int> compressed_scope_sizes);
	Scope(Scope* original);
	~Scope();
	void activate(std::vector<std::vector<double>>& flat_vals,
				  std::vector<double> inputs,
				  std::vector<double>& outputs,
				  double& predicted_score);
	void backprop(std::vector<double> input_errors,
				  std::vector<double>& output_errors);
};

Scope* construct_scope(std::vector<Node*> nodes);

#endif /* SCOPE_H */