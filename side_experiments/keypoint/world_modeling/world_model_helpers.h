#ifndef WORLD_MODEL_HELPERS_H
#define WORLD_MODEL_HELPERS_H

class Keypoint;
class WorldModel;

bool potential_keypoint_is_duplicate(WorldModel* world_model,
									 Keypoint* potential);

#endif /* WORLD_MODEL_HELPERS_H */