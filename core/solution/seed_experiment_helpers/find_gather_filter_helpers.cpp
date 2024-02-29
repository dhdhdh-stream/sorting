#include "seed_experiment.h"

using namespace std;

void SeedExperiment::find_gather_filter_backprop(double target_val,
												 SeedExperimentOverallHistory* history) {
	/**
	 * - don't check history->has_target, i.e., if curr_filter is hit
	 *   - meant to capture all gathers/filters
	 *     - for comparison against new existing score
	 */

	this->curr_gather_non_seed_score += target_val;

	this->sub_state_iter++;
	if (this->sub_state_iter >= FIND_GATHER_NUM_SAMPLES_PER_ITER) {
		this->o_target_val_histories.reserve(FIND_GATHER_NUM_SAMPLES_PER_ITER);

		this->state = SEED_EXPERIMENT_STATE_FIND_GATHER_EXISTING;
		/**
		 * - leave this->state_iter unchanged
		 */
		this->sub_state_iter = 0;
	}
}
