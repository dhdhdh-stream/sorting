#ifndef NODE_H
#define NODE_H

class Node {
public:
	int new_layer_size;	// TODO: don't do anything if == 0
	Network* obs_network;

	// input layers get retrained on branching, rebuilt on compound actions
	std::vector<int> score_input_layer;
	std::vector<int> score_input_sizes;
	std::vector<Network*> score_input_networks;	// includes just added layer
	Network* score_network;	// score diff, not full score

	std::vector<int> input_layer;	// take from this, and update +1
	std::vector<int> input_sizes;
	std::vector<Network*> input_networks;

	Network* compression_network;
	int compress_num_layers;
	int compress_size;
	vector<int> compressed_scope_sizes;

	// no scopes_on, but then should normalize inputs
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<double>& obs,
				  double& predicted_score);
	void backprop(std::vector<std::vector<double>>& state_errors,
				  double& score_error);
};

#endif /* NODE_H */