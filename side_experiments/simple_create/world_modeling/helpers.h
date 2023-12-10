#ifndef HELPERS_H
#define HELPERS_H

#include <vector>

class HiddenState;

void create_experiment(std::vector<HiddenState*>& state_history,
					   std::vector<int>& action_history);

#endif /* HELPERS_H */