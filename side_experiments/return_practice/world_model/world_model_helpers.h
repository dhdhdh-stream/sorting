#ifndef WORLD_MODEL_HELPERS_H
#define WORLD_MODEL_HELPERS_H

#include <vector>

class Network;
class ProblemType;
class WorldModel;
class Wrapper;

void obs_helper(std::vector<double>& obs,
				std::vector<double>& state,
				Wrapper* wrapper);
void action_helper(int action,
				   std::vector<double>& state,
				   Wrapper* wrapper);

void init_helper(ProblemType* problem_type,
				 Wrapper* wrapper);

void update_world_model_helper(std::vector<std::vector<double>>& obs,
							   std::vector<int>& actions,
							   double target_val,
							   Wrapper* wrapper);
void no_state_update_world_model_helper(std::vector<std::vector<double>>& obs,
										std::vector<int>& actions,
										double target_val,
										Wrapper* wrapper);

void train_helper(Wrapper* wrapper);

void force_sequence_helper(Wrapper* wrapper);
void large_random(Wrapper* wrapper);

#endif /* WORLD_MODEL_HELPERS_H */