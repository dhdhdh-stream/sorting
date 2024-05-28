#ifndef NN_HELPERS_H
#define NN_HELPERS_H

#include <vector>

class Network;

void train_network(std::vector<std::vector<double>>& inputs,
				   std::vector<double>& target_vals,
				   std::vector<int>& test_input_indexes,
				   Network* network);
/**
 * - can remove existing inputs as well
 */

void measure_network(std::vector<std::vector<double>>& inputs,
					 std::vector<double>& target_vals,
					 Network* network,
					 double& average_misguess,
					 double& misguess_standard_deviation);

void optimize_network(std::vector<std::vector<double>>& inputs,
					  std::vector<double>& target_vals,
					  Network* network);

#endif /* NN_HELPERS_H */