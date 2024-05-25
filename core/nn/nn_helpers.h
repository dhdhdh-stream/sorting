#ifndef NN_HELPERS_H
#define NN_HELPERS_H

#include <vector>

class AbstractNode;
class Network;
class Scope;

void train_network(std::vector<std::vector<std::vector<double>>>& inputs,
				   std::vector<double>& target_vals,
				   std::vector<std::vector<Scope*>>& test_input_scope_contexts,
				   std::vector<std::vector<AbstractNode*>>& test_input_node_contexts,
				   std::vector<int>& test_input_obs_indexes,
				   Network* network);
void train_network(std::vector<std::vector<std::vector<double>>>& inputs,
				   std::vector<double>& target_vals,
				   std::vector<AbstractNode*>& test_input_node_contexts,
				   std::vector<int>& test_input_obs_indexes,
				   Network* network);

void measure_network(std::vector<std::vector<std::vector<double>>>& inputs,
					 std::vector<double>& target_vals,
					 Network* network,
					 double& average_misguess,
					 double& misguess_standard_deviation);

void optimize_network(std::vector<std::vector<std::vector<double>>>& inputs,
					  std::vector<double>& target_vals,
					  Network* network);

#endif /* NN_HELPERS_H */