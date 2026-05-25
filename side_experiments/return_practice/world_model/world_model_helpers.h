#ifndef WORLD_MODEL_HELPERS_H
#define WORLD_MODEL_HELPERS_H

#include <vector>

#include "run.h"

class Network;
class WorldModel;
class WorldModelWrapper;

void obs_helper(std::vector<double>& obs,
				std::vector<double>& state,
				WorldModelWrapper* wrapper);
void action_helper(int action,
				   std::vector<double>& state,
				   WorldModelWrapper* wrapper);

double predict_helper(std::vector<double>& existing_state,
					  std::vector<int>& potential_return,
					  WorldModelWrapper* wrapper);

void init_run(Run& run,
			  WorldModelWrapper* wrapper);

void explore_obs_step(std::vector<double>& obs,
					  Run& run,
					  WorldModelWrapper* wrapper);

void explore_action_step(Run& run,
						 int& next_action,
						 bool& is_done,
						 WorldModelWrapper* wrapper);

void train_helper(WorldModelWrapper* wrapper);

#endif /* WORLD_MODEL_HELPERS_H */