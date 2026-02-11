#ifndef BUILD_NETWORK_HELPERS_H
#define BUILD_NETWORK_HELPERS_H

#include <vector>

class BuildNetwork;

void update_network(std::vector<std::vector<double>>& obs_histories,
					std::vector<double>& target_val_histories,
					BuildNetwork*& network);

#endif /* BUILD_NETWORK_HELPERS_H */