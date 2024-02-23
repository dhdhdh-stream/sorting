#include "seed_experiment.h"

#include <cmath>

#include "seed_experiment_filter.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_SAMPLES_PER_ITER = 2;
#else
const int NUM_SAMPLES_PER_ITER = 20;
#endif /* MDEBUG */

void SeedExperiment::find_filter_backprop(double target_val,
										  SeedExperimentOverallHistory* history) {
	/**
	 * - don't need to check history->has_target
	 */

	this->curr_filter_score += target_val;

	this->sub_state_iter++;
	if (this->sub_state_iter >= NUM_SAMPLES_PER_ITER) {
		this->curr_filter_score /= NUM_SAMPLES_PER_ITER;
		#if defined(MDEBUG) && MDEBUG
		if (true) {
		#else
		double score_diff = this->curr_filter_score - this->existing_average_score;
		double t_score = score_diff
			/ (this->existing_score_standard_deviation / sqrt(NUM_SAMPLES_PER_ITER));

		if (t_score > -0.2) {
		#endif /* MDEBUG */
			this->curr_filter_score = 0.0;

			this->state = SEED_EXPERIMENT_STATE_VERIFY_1ST_FILTER;
			/**
			 * - leave this->state_iter unchanged
			 */
			this->sub_state_iter = 0;
		} else {
			delete this->curr_filter;
			this->curr_filter = NULL;

			this->state_iter++;
			if (this->state_iter >= FIND_FILTER_ITER_LIMIT) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				create_filter();
				this->sub_state_iter = 0;
			}
		}
	}
}
