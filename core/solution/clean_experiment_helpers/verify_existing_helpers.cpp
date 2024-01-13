#include "clean_experiment.h"

#include "constants.h"
#include "globals.h"
#include "solution.h"

using namespace std;

void CleanExperiment::verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->existing_score += target_val;

	if (!run_helper.exceeded_limit) {
		if (run_helper.max_depth > solution->max_depth) {
			solution->max_depth = run_helper.max_depth;

			if (solution->max_depth < 50) {
				solution->depth_limit = solution->max_depth + 10;
			} else {
				solution->depth_limit = (int)(1.2*(double)solution->max_depth);
			}
		}
	}

	this->state_iter++;
	if (this->state == CLEAN_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
			&& this->state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		this->existing_score /= (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		this->state = CLEAN_EXPERIMENT_STATE_VERIFY_1ST;
		this->state_iter = 0;
	} else if (this->state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		this->existing_score /= (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		this->state = CLEAN_EXPERIMENT_STATE_VERIFY_2ND;
		this->state_iter = 0;
	}
}
