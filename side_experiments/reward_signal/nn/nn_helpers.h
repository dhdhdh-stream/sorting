#ifndef NN_HELPERS_H
#define NN_HELPERS_H

#include <vector>

class Network;

void train_network(std::vector<std::vector<double>>& inputs,
				   std::vector<std::vector<bool>>& input_is_on,
				   std::vector<double>& target_vals,
				   Network* network);

void measure_network(std::vector<std::vector<double>>& inputs,
					 std::vector<std::vector<bool>>& input_is_on,
					 std::vector<double>& target_vals,
					 Network* network,
					 double& average_misguess,
					 double& misguess_standard_deviation);

void measure_predict_network(std::vector<std::vector<double>>& inputs,
							 std::vector<std::vector<bool>>& input_is_on,
							 std::vector<double>& target_vals,
							 Network* network,
							 double& average_misguess,
							 double& val_standard_deviation);

void optimize_network(std::vector<std::vector<double>>& inputs,
					  std::vector<std::vector<bool>>& input_is_on,
					  std::vector<double>& target_vals,
					  Network* network);

#endif /* NN_HELPERS_H */