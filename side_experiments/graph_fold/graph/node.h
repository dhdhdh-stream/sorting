#ifndef NODE_H
#define NODE_H

#include <vector>

#include "fold_helper.h"
#include "network.h"

class Node {
public:
	int state_size;

	Network* network;

	Node(int state_size);
	~Node();

	void activate(double obs,
				  double* state_vals);
	void activate(double obs,
				  double* state_vals,
				  std::vector<NetworkHistory*>& network_historys);
	void activate_greedy(double obs,
						 double* state_vals);
	void activate_greedy(double obs,
						 double* state_vals,
						 std::vector<NetworkHistory*>& network_historys);
	void backprop(double* state_errors,
				  std::vector<NetworkHistory*>& network_historys);
	void backprop_errors_with_no_weight_change(
		double* state_errors,
		std::vector<NetworkHistory*>& network_historys);
	void backprop_greedy(double* state_errors,
						 std::vector<NetworkHistory*>& network_historys);
	void backprop_greedy_errors_with_no_weight_change(
		double* state_errors,
		std::vector<NetworkHistory*>& network_historys);
};

#endif /* NODE_H */