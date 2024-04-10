#include "pass_through_experiment.h"

#include "constants.h"
#include "globals.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::experiment_verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	/**
	 * - has to be root, so this->parent_experiment == NULL
	 */
	if (!run_helper.exceeded_limit) {
		if (run_helper.max_depth > solution->max_depth) {
			solution->max_depth = run_helper.max_depth;
		}
	}

	if (this->root_state == ROOT_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
			&& (int)this->o_target_val_histories.size() >= VERIFY_1ST_NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / VERIFY_1ST_NUM_DATAPOINTS;

		this->o_target_val_histories.clear();

		this->o_target_val_histories.reserve(VERIFY_1ST_NUM_DATAPOINTS);

		this->root_state = ROOT_EXPERIMENT_STATE_VERIFY_1ST;
	} else if ((int)this->o_target_val_histories.size() >= VERIFY_2ND_NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / VERIFY_2ND_NUM_DATAPOINTS;

		this->o_target_val_histories.clear();

		this->o_target_val_histories.reserve(VERIFY_2ND_NUM_DATAPOINTS);

		this->root_state = ROOT_EXPERIMENT_STATE_VERIFY_2ND;
	}
}
