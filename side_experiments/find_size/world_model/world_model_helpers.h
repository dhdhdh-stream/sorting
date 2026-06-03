#ifndef WORLD_MODEL_HELPERS_H
#define WORLD_MODEL_HELPERS_H

#include <vector>

class WorldModel;

void obs_helper(std::vector<double>& obs,
				std::vector<double>& state,
				WorldModel* world_model);
void action_helper(int action,
				   std::vector<double>& state,
				   WorldModel* world_model);

void update_world_model_helper(std::vector<std::vector<double>>& obs,
							   std::vector<int>& actions,
							   double target_val,
							   double& error,
							   WorldModel* world_model);

void measure_test(WorldModel* world_model);

#endif /* WORLD_MODEL_HELPERS_H */