#ifndef ACTION_HELPERS_H
#define ACTION_HELPERS_H

class PredictedWorld;
class WorldTruth;

void apply_action(WorldTruth& world_truth,
				  int action,
				  int& new_obs);
void update_predicted(PredictedWorld& predicted_world,
					  int action,
					  int new_obs);

#endif /* ACTION_HELPERS_H */