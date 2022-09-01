#ifndef NODE_H
#define NODE_H

#include <vector>

#include "fold_helper.h"
#include "network.h"

class Node {
public:
	int state_size;

	bool network_on;
	Network* network;

	Node(int state_size);
	~Node();

	void activate(double obs,
				  double* state_vals,
				  std::vector<NetworkHistory*>& network_historys);
	void backprop(double* state_errors,
				  std::vector<NetworkHistory*>& network_historys);
};

#endif /* NODE_H */