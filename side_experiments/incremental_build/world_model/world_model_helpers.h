#ifndef WORLD_MODEL_HELPERS_H
#define WORLD_MODEL_HELPERS_H

#include <vector>

class WorldModel;
class Wrapper;

void obs_helper(std::vector<double>& obs,
				std::vector<double>& state,
				Wrapper* wrapper);
void action_helper(int action,
				   std::vector<double>& state,
				   Wrapper* wrapper);

void update_world_model_helper(std::vector<std::vector<double>>& obs,
							   std::vector<int>& actions,
							   double target_val,
							   WorldModel* world_model,
							   Wrapper* wrapper);

void temp_train_helper(std::vector<std::vector<std::vector<double>>>& train_obs,
					   std::vector<std::vector<int>>& train_actions,
					   std::vector<double>& train_target_vals,
					   WorldModel* potential_world_model,
					   Wrapper* wrapper);
void train_helper(Wrapper* wrapper);

void measure_test(Wrapper* wrapper);

#endif /* WORLD_MODEL_HELPERS_H */