#ifndef RUN_HELPERS_H
#define RUN_HELPERS_H

#include <vector>

class WorldModel;

void train_model(WorldModel* world_model);

double measure_model(WorldModel* world_model);

#endif /* RUN_HELPERS_H */