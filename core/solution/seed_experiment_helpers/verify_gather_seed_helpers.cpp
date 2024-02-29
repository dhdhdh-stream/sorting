#include "seed_experiment.h"

#include "constants.h"
#include "globals.h"
#include "solution.h"

using namespace std;

void SeedExperiment::verify_gather_seed_backprop(double target_val,
												 SeedExperimentOverallHistory* history) {
	if (history->has_target) {
		this->curr_gather_seed_score += target_val;
		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (target_val > this->existing_average_score + this->existing_score_standard_deviation) {
		#endif /* MDEBUG */
			this->curr_gather_is_higher++;
		}

		this->sub_state_iter++;
		if (this->state == SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER_SEED
				&& this->sub_state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
			this->state = SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER_FILTER;
			/**
			 * - leave this->state_iter unchanged
			 */
			this->sub_state_iter = 0;
		} else if (this->sub_state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
			this->state = SEED_EXPERIMENT_STATE_VERIFY_2ND_GATHER_FILTER;
			/**
			 * - leave this->state_iter unchanged
			 */
			this->sub_state_iter = 0;
		}
	}
}
