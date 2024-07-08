#ifndef NN_HELPERS_H
#define NN_HELPERS_H

#include <vector>

class Network;

void train_network(std::vector<std::vector<std::vector<double>>>& inputs,
				   std::vector<double>& target_vals,
				   Network* network);

#endif /* NN_HELPERS_H */