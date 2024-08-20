#ifndef UPDATE_HELPERS_H
#define UPDATE_HELPERS_H

#include <vector>

class WorldModel;

void update(WorldModel* world_model,
			std::vector<std::vector<double>>& obs,
			std::vector<std::vector<int>>& actions);

#endif /* UPDATE_HELPERS_H */