#ifndef HELPERS_H
#define HELPERS_H

#include <vector>

class Action;
class WorldState;

void create_experiment(std::vector<WorldState*>& state_history,
					   std::vector<int>& sequence_index_history,
					   std::vector<Action*>& action_sequence,
					   std::vector<std::vector<int>>& action_state_sequence);

#endif /* HELPERS_H */