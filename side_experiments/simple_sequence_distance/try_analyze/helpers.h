#ifndef HELPERS_H
#define HELPERS_H

#include <utility>
#include <vector>

class Try;

void compare_tries(Try* original,
				   Try* potential,
				   int& distance,
				   std::vector<std::pair<int, int>>& diffs);

#endif /* HELPERS_H */