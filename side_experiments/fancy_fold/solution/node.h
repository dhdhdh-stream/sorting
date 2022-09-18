#ifndef NODE_H
#define NODE_H

#include <fstream>
#include <vector>

#include "network.h"

class Node {
public:
	int id;

	bool outputs_state;

	bool update_existing_scope;
	int new_scope_size;	// matters when handling multiple observations at once
	Network* state_network;

	std::vector<Network*> compression_networks;

	Node(int id,
		 bool outputs_state,
		 bool update_existing_scope,
		 int new_scope_size,
		 Network* state_network,
		 std::vector<Network*> compression_networks);
	Node(int id,
		 std::ifstream& input_file);
	~Node();
	void activate(std::vector<std::vector<double>>& state_vals,
				  double observation);
	void activate_zero_train(std::vector<std::vector<double>>& state_vals,
							 double observation,
							 std::vector<std::vector<double>>& zero_train_state_vals);
	// void backprop(std::vector<std::vector<double>>& state_errors);

	void get_scope_sizes(std::vector<int>& scope_sizes);

	void save(std::ofstream& output_file);
};

#endif /* NODE_H */