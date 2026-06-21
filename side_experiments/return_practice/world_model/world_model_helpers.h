#ifndef WORLD_MODEL_HELPERS_H
#define WORLD_MODEL_HELPERS_H

#include <vector>

class ExperimentRun;
class PredictWrapper;
class ProblemType;
class WorldModel;
class Wrapper;

void obs_helper(std::vector<double>& obs,
				std::vector<double>& state,
				Wrapper* wrapper);
void obs_helper_w_history(std::vector<double>& obs,
						  ExperimentRun* run);
void action_helper(int action,
				   std::vector<double>& state,
				   Wrapper* wrapper);
void action_helper_w_history(int action,
							 ExperimentRun* run);
void predict_helper(std::vector<double>& state,
					Wrapper* wrapper);

const int ITERS_PER_PREDICT = 40;

#if defined(MDEBUG) && MDEBUG
const int PREDICT_CANDIDATE_CHECK_NUM_ITERS = 100;
#else
const int PREDICT_CANDIDATE_CHECK_NUM_ITERS = 2000000;
/**
 * - actual num iters divided by ITERS_PER_PREDICT
 */
#endif /* MDEBUG */
void predict_update_helper(std::vector<double>& start_state,
						   std::vector<double>& end_state,
						   PredictWrapper* predict_wrapper,
						   Wrapper* wrapper);

void init_helper(ProblemType* problem_type,
				 Wrapper* wrapper);

void update_world_model_helper(Wrapper* wrapper);
void update_helper(double target_val,
				   ExperimentRun* run);

void view_world_model_helper(Wrapper* wrapper);

#endif /* WORLD_MODEL_HELPERS_H */