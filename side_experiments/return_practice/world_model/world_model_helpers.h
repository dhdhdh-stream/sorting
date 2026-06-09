#ifndef WORLD_MODEL_HELPERS_H
#define WORLD_MODEL_HELPERS_H

#include <vector>

class ProblemType;
class WorldModel;
class Wrapper;

void obs_helper(std::vector<double>& obs,
				std::vector<double>& state,
				Wrapper* wrapper);
void action_helper(int action,
				   std::vector<double>& state,
				   Wrapper* wrapper);
void predict_helper(int action,
					std::vector<double>& state,
					Wrapper* wrapper);
void predict_obs_helper(std::vector<double>& state,
						Wrapper* wrapper);

void init_helper(ProblemType* problem_type,
				 Wrapper* wrapper);

void update_world_model_helper(Wrapper* wrapper);
void check_state_size_helper(Wrapper* wrapper);

#endif /* WORLD_MODEL_HELPERS_H */