#ifndef LOCALIZE_HELPERS_H
#define LOCALIZE_HELPERS_H

class WorldModel;
class WorldTruth;

bool localize(WorldTruth* world_truth,
			  WorldModel* world_model);

#endif /* LOCALIZE_HELPERS_H */