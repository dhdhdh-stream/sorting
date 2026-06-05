#include "world_model_helpers.h"

#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INIT_ITERS = 100;
#else
const int INIT_ITERS = 1000000;
#endif /* MDEBUG */

void init_helper(ProblemType* problem_type,
				 Wrapper* wrapper) {
	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, wrapper->num_actions-1);
	for (int iter_index = 0; iter_index < INIT_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		vector<vector<double>> curr_obs;
		vector<int> curr_actions;

		curr_obs.push_back(problem->get_observations());

		int num_actions = num_actions_distribution(generator);
		for (int a_index = 0; a_index < num_actions; a_index++) {
			int action = action_distribution(generator);

			problem->perform_action(action);
			curr_actions.push_back(action);

			curr_obs.push_back(problem->get_observations());
		}

		double target_val = problem->score_result();

		if (wrapper->sample_obs.size() < SAMPLES_NUM_SAVE) {
			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(target_val);
		} else {
			wrapper->sample_obs[wrapper->sample_index] = curr_obs;
			wrapper->sample_actions[wrapper->sample_index] = curr_actions;
			wrapper->sample_target_vals[wrapper->sample_index] = target_val;
		}
		wrapper->sample_index++;
		if (wrapper->sample_index >= SAMPLES_NUM_SAVE) {
			check_state_size_helper(wrapper);

			wrapper->sample_index = 0;
		}

		update_world_model_helper(wrapper);

		delete problem;
	}
}
