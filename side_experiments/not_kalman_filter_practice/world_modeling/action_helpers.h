#ifndef ACTION_HELPERS_H
#define ACTION_HELPERS_H

#include "predicted_world.h"
#include "world_truth.h"

const int ACTION_LEFT = 0;
const int ACTION_UP = 1;
const int ACTION_RIGHT = 2;
const int ACTION_DOWN = 3;

void apply_action(WorldTruth& world_truth,
				  int action,
				  int& new_obs);
void update_predicted(PredictedWorld& predicted_world,
					  int action,
					  int new_obs);

#endif /* ACTION_HELPERS_H */