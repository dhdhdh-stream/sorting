#ifndef BREAKDOWN_HELPERS_H
#define BREAKDOWN_HELPERS_H

#include <utility>
#include <vector>

void init_breakdown(std::vector<int>& solution,
					std::vector<int>& lower_bounds,
					std::vector<int>& upper_bounds,
					std::vector<std::pair<int,int>>& splits);

#endif /* BREAKDOWN_HELPERS_H */