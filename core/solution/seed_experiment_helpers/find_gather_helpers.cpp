#include "seed_experiment.h"

#include <cmath>

#include "seed_experiment_filter.h"
#include "seed_experiment_gather.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_SAMPLES_PER_ITER = 4;
#else
const int NUM_SAMPLES_PER_ITER = 40;
#endif /* MDEBUG */

void SeedExperiment::find_gather_backprop(double target_val,
										  SeedExperimentOverallHistory* history) {
	if (history->has_target) {
		if (this->sub_state_iter != -1) {
			if (this->sub_state_iter%2 == 0) {
				if (target_val > this->existing_average_score + this->existing_score_standard_deviation) {
					this->curr_gather_is_higher++;
				}
			} else {
				this->curr_gather_score += target_val;
			}
		}

		this->sub_state_iter++;
		if (this->sub_state_iter >= NUM_SAMPLES_PER_ITER) {
			double new_higher_ratio = (double)this->curr_gather_is_higher / (double)(NUM_SAMPLES_PER_ITER / 2);
			this->curr_gather_score /= (NUM_SAMPLES_PER_ITER / 2);

			double ratio_standard_deviation = sqrt(this->curr_higher_ratio * (1 - this->curr_higher_ratio));
			double ratio_t_score = (new_higher_ratio - this->curr_higher_ratio)
				/ (ratio_standard_deviation / sqrt(NUM_SAMPLES_PER_ITER / 2));

			double score_diff = this->curr_gather_score - this->existing_average_score;
			double t_score = score_diff
				/ (this->existing_score_standard_deviation / sqrt(NUM_SAMPLES_PER_ITER / 2));

			if (ratio_t_score > -0.2 && t_score > -0.2) {
				this->curr_gather_is_higher = 0;
				this->curr_gather_score = 0.0;

				this->state = SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER;
				/**
				 * - leave this->state_iter unchanged
				 */
				this->sub_state_iter = 0;
			} else {
				if (this->curr_gather != NULL) {
					delete this->curr_gather;
					this->curr_gather = NULL;
				}

				this->state_iter++;
				if (this->state_iter >= FIND_GATHER_ITER_LIMIT) {
					if (this->curr_filter_is_success) {
						this->curr_filter->add_to_scope();
						this->filters.push_back(this->curr_filter);
					} else {
						delete this->curr_filter;
					}
					this->curr_filter = NULL;

					if (this->filter_step_index == (int)this->best_step_types.size()) {
						this->result = EXPERIMENT_RESULT_FAIL;
					} else {
						this->state = SEED_EXPERIMENT_STATE_FIND_FILTER;
						this->state_iter = 0;
						create_filter();
						this->sub_state_iter = 0;
					}
				} else {
					this->sub_state_iter = -1;
				}
			}
		}
	}
}
