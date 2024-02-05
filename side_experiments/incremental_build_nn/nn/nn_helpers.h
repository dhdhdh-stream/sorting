#ifndef NN_HELPERS_H
#define NN_HELPERS_H

void train_network(std::vector<std::vector<std::vector<double>>>& inputs,
				   std::vector<double>& target_vals,
				   std::vector<Scope*>& test_input_scope_contexts,
				   std::vector<AbstractNode*>& test_input_node_contexts,
				   Network* network);

void measure_network(std::vector<std::vector<std::vector<double>>>& inputs,
					 std::vector<double>& target_vals,
					 Network* network,
					 double& average_misguess,
					 double& misguess_variance);

void optimize_network(std::vector<std::vector<std::vector<double>>>& inputs,
					  std::vector<double>& target_vals,
					  Network* network);

#endif /* NN_HELPERS_H */