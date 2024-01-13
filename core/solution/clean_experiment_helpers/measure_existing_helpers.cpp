#include "clean_experiment.h"

#include "globals.h"
#include "solution.h"

using namespace std;

void CleanExperiment::measure_existing_backprop(
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
	if (this->state_iter >= solution->curr_num_datapoints) {
		this->existing_score /= solution->curr_num_datapoints;

		this->state = CLEAN_EXPERIMENT_STATE_MEASURE_NEW;
		this->state_iter = 0;
	}
}
