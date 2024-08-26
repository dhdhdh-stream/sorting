#ifndef LOCALIZE_HELPERS_H
#define LOCALIZE_HELPERS_H

#include <vector>

class WorldModel;
class WorldTruth;

bool localize(WorldTruth* world_truth,
			  WorldModel* world_model,
			  std::vector<int>& unknown_actions);

#endif /* LOCALIZE_HELPERS_H */