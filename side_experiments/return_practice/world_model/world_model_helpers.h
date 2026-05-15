#ifndef WORLD_MODEL_HELPERS_H
#define WORLD_MODEL_HELPERS_H

#include <vector>

#include "run.h"

class WorldModelWrapper;

double predict_helper(std::vector<double>& existing_state,
					  std::vector<int>& potential_return,
					  WorldModelWrapper* wrapper);

void explore_obs_step(std::vector<double>& obs,
					  Run& run,
					  WorldModelWrapper* wrapper);

void explore_action_step(Run& run,
						 int& next_action,
						 bool& is_done,
						 WorldModelWrapper* wrapper);

#endif /* WORLD_MODEL_HELPERS_H */