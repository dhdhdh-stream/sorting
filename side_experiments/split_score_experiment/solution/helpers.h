#ifndef HELPERS_H
#define HELPERS_H

#include <vector>

class Network;

bool split_helper(std::vector<std::vector<double>>& existing_vals,
				  std::vector<std::vector<double>>& explore_vals,
				  Network* match_network);

bool train_score(std::vector<std::vector<double>>& vals,
				 std::vector<double>& target_vals,
				 Network* network);

double calc_miss_average_guess(std::vector<std::vector<double>>& vals,
							   std::vector<double>& target_vals,
							   std::vector<Network*>& match_networks);

double calc_signal(std::vector<double>& vals,
				   std::vector<Network*>& match_networks,
				   std::vector<Network*>& signal_networks,
				   double miss_average_guess);

void eval_signal(std::vector<std::vector<double>>& vals,
				 std::vector<double>& target_vals,
				 std::vector<Network*>& match_networks,
				 std::vector<Network*>& signal_networks,
				 double miss_average_guess,
				 double& new_misguess,
				 double& new_misguess_standard_deviation);

bool alternate_train_helper(std::vector<std::vector<double>>& vals,
							std::vector<double>& target_vals,
							std::vector<double>& existing_predicted_vals,
							Network* match_network,
							Network* score_network);

#endif /* HELPERS_H */