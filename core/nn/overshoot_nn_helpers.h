#ifndef OVERSHOOT_NN_HELPERS_H
#define OVERSHOOT_NN_HELPERS_H

#include <vector>

class AbstractNode;
class Network;
class Scope;

void overshoot_train_network(std::vector<std::vector<std::vector<double>>>& inputs,
							 std::vector<double>& polarities,
							 std::vector<std::vector<Scope*>>& test_input_scope_contexts,
							 std::vector<std::vector<AbstractNode*>>& test_input_node_contexts,
							 Network* network);

void overshoot_measure_network(std::vector<std::vector<std::vector<double>>>& inputs,
							   std::vector<double>& polarities,
							   Network* network,
							   double& average_misguess,
							   double& misguess_variance);

void overshoot_optimize_network(std::vector<std::vector<std::vector<double>>>& inputs,
								std::vector<double>& polarities,
								Network* network);

#endif /* OVERSHOOT_NN_HELPERS_H */