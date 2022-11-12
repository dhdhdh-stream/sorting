#ifndef SCOPE_H
#define SCOPE_H

class Scope : public AbstractScope {
public:
	std::vector<AbstractScope*> actions;

	int num_inputs;
	int num_outputs;

	std::vector<std::vector<int>> scope_input_sizes;
	std::vector<std::vector<SmallNetwork*>> scope_input_networks;
	std::vector<int> scope_compressed_scope_sizes;
	// use all inputs at start, so scope_compressed_s_input_sizes is just num_inputs
	// during test_node, don't compress s_input unless have to

	std::vector<int> new_layer_sizes;	// 0 or 1
	// no obs_networks for sorting

	std::vector<bool> is_simple_append;

	std::vector<bool> is_loose;

	std::vector<bool> is_branch;
	std::vector<Branch*> branches;

	std::vector<SmallNetwork*> score_networks;

	std::vector<std::vector<int>> inner_input_sizes;
	std::vector<std::vector<SmallNetwork*>> inner_input_networks;

	std::vector<int> inner_compress_num_layers;
	std::vector<int> inner_compress_new_sizes;
	// add 2 compress 1, or add new state + compress for 1st, or scope compress
	std::vector<Network*> inner_compression_networks;
	std::vector<std::vector<int>> inner_compressed_scope_sizes;

	std::vector<int> end_compressed_scope_sizes;	// earliest to latest

	// for inner scope, sum all of its explore_weights, for branches, average
	std::vector<double> explore_weights;

	// no outputs, pass in vector instead
	// std::vector<double> outputs;

	int explore_index;
	bool explore_inner;
	std::vector<AbstractAction*> explore_path;
	FoldNetwork* explore_fold_network;
	std::vector<Network*> explore_scope_input_networks;
	std::vector<FinishedStep*> steps;
	InProgressStep* curr_step;
};

#endif /* SCOPE_H */