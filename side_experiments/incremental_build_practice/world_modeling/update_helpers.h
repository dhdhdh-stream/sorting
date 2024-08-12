#ifndef UPDATE_HELPERS_H
#define UPDATE_HELPERS_H

#include <vector>

class WorldModel;

void update(WorldModel* world_model,
			std::vector<std::vector<double>>& obs,
			std::vector<std::vector<int>>& actions);

void update(WorldModel* world_model,
			std::vector<std::vector<double>>& obs,
			std::vector<std::vector<int>>& actions,
			int starting_state_index,
			std::vector<int>& new_state_indexes,
			int ending_state_index,
			int starting_action,
			std::vector<int>& new_actions,
			int ending_action);

#endif /* UPDATE_HELPERS_H */