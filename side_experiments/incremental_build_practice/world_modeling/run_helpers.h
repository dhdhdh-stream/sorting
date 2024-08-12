#ifndef RUN_HELPERS_H
#define RUN_HELPERS_H

#include <vector>

class WorldModel;

void train_model(WorldModel* world_model);

void train_model(WorldModel* world_model,
				 int starting_state_index,
				 std::vector<int>& new_state_indexes,
				 int ending_state_index,
				 int starting_action,
				 std::vector<int>& new_actions,
				 int ending_action);

double measure_model(WorldModel* world_model);

#endif /* RUN_HELPERS_H */