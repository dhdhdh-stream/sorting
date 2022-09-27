#ifndef NODE_H
#define NODE_H

#include <fstream>
#include <vector>

#include "compression_network.h"
#include "state_network.h"

class Node {
public:
	std::string id;

	bool outputs_state;

	bool update_existing_scope;
	int new_scope_size;	// matters when handling multiple observations at once
	StateNetwork* state_network;

	std::vector<CompressionNetwork*> compression_networks;
	std::vector<int> compressed_scope_sizes;

	Node(std::string id,
		 bool outputs_state,
		 bool update_existing_scope,
		 int new_scope_size,
		 StateNetwork* state_network,
		 std::vector<CompressionNetwork*> compression_networks,
		 std::vector<int> compressed_scope_sizes);
	Node(std::string id,
		 std::ifstream& input_file);
	~Node();
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  double observation);
	void backprop(std::vector<std::vector<double>>& state_errors);
	void calc_max_update(double& max_update,
						 double learning_rate);
	void update_weights(double factor,
						double learning_rate);

	void get_scope_sizes(std::vector<int>& scope_sizes);

	void save(std::ofstream& output_file);
};

#endif /* NODE_H */