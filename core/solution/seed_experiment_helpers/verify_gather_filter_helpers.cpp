#include "seed_experiment.h"

#include "constants.h"
#include "globals.h"
#include "solution.h"

using namespace std;

void SeedExperiment::verify_gather_filter_backprop(double target_val,
												   SeedExperimentOverallHistory* history) {
	this->curr_gather_non_seed_score += target_val;

	this->sub_state_iter++;
	if (this->state == SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER_FILTER
			&& this->sub_state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		this->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		this->state = SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER_EXISTING;
		/**
		 * - leave this->state_iter unchanged
		 */
		this->sub_state_iter = 0;
	} else if (this->sub_state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		this->state = SEED_EXPERIMENT_STATE_VERIFY_2ND_GATHER_EXISTING;
		/**
		 * - leave this->state_iter unchanged
		 */
		this->sub_state_iter = 0;
	}
}
