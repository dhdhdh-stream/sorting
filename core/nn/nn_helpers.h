#ifndef NN_HELPERS_H
#define NN_HELPERS_H

#include <vector>

class FamiliarityNetwork;
class Network;

void train_network(std::vector<std::vector<double>>& inputs,
				   std::vector<double>& target_vals,
				   Network* network);
void train_w_drop_network(std::vector<std::vector<double>>& inputs,
						  std::vector<double>& target_vals,
						  Network* network);

void measure_network(std::vector<std::vector<double>>& inputs,
					 std::vector<double>& target_vals,
					 Network* network,
					 double& average_misguess,
					 double& misguess_standard_deviation);

void optimize_network(std::vector<std::vector<double>>& inputs,
					  std::vector<double>& target_vals,
					  Network* network);
void optimize_w_drop_network(std::vector<std::vector<double>>& inputs,
							 std::vector<double>& target_vals,
							 Network* network);

void train_familiarity_network(std::vector<std::vector<double>>& inputs,
							   std::vector<double>& target_vals,
							   FamiliarityNetwork* network);
void measure_familiarity_network(std::vector<std::vector<double>>& inputs,
								 std::vector<double>& target_vals,
								 FamiliarityNetwork* network,
								 double& average_misguess,
								 double& misguess_standard_deviation);

#endif /* NN_HELPERS_H */