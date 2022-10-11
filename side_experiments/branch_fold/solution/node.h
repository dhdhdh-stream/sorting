#ifndef NODE_H
#define NODE_H

#include <vector>

#include "network.h"

class Node {
public:
	std::string id;

	int new_layer_size;
	Network* obs_network;

	// input layers get retrained on branching, rebuilt on compound actions
	std::vector<int> score_input_layer;
	std::vector<int> score_input_sizes;
	std::vector<Network*> score_input_networks;
	Network* score_network;

	int compress_num_layers;
	int compress_new_size;
	std::vector<int> input_layer;
	std::vector<int> input_sizes;
	std::vector<Network*> input_networks;
	Network* compression_network;
	std::vector<int> compressed_scope_sizes;	// earliest to latest

	Node(std::string id,
		 int new_layer_size,
		 Network* obs_network,
		 std::vector<int> score_input_layer,
		 std::vector<int> score_input_sizes,
		 std::vector<Network*> score_input_networks,
		 Network* score_network,
		 int compress_num_layers,
		 int compress_new_size,
		 std::vector<int> input_layer,
		 std::vector<int> input_sizes,
		 std::vector<Network*> input_networks,
		 Network* compression_network,
		 std::vector<int> compressed_scope_sizes);
	Node(std::ifstream& input_file);
	~Node();
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<double>& obs,
				  double& predicted_score);
	void backprop(std::vector<std::vector<double>>& state_errors,
				  double& score_error);

	void get_scope_sizes(std::vector<int>& scope_sizes);

	void save(std::ofstream& output_file);
};

#endif /* NODE_H */