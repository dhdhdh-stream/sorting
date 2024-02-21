#include "seed_experiment.h"

using namespace std;

void SeedExperiment::verify_filter_backprop(double target_val,
											SeedExperimentOverallHistory* history) {
	/**
	 * - don't need to check history->has_target
	 */

	this->curr_filter_score += target_val;

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	this->sub_state_iter++;
	if (this->state == SEED_EXPERIMENT_STATE_VERIFY_1ST_FILTER
			&& this->sub_state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		this->curr_filter_score /= (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double score_diff = this->curr_filter_score - this->existing_average_score;
		double t_score = score_diff
			/ (this->existing_score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));

		if (t_score > -0.2) {
		#endif /* MDEBUG */
			this->curr_filter_score = 0.0;

			this->state = SEED_EXPERIMENT_STATE_VERIFY_2ND_FILTER;
			/**
			 * - leave this->state_iter unchanged
			 */
			this->sub_state_iter = 0;
		} else {
			this->curr_filter->clean_fail();
			delete this->curr_filter;
			this->curr_filter = NULL;

			this->state_iter++;
			if (this->state_iter >= FIND_FILTER_ITER_LIMIT) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->state = SEED_EXPERIMENT_STATE_FIND_FILTER;
				create_filter();
				this->sub_state_iter = 0;
			}
		}
	} else if (this->sub_state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		this->curr_filter_score /= (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double score_diff = this->curr_filter_score - this->existing_average_score;
		double t_score = score_diff
			/ (this->existing_score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));

		if (t_score > -0.2) {
		#endif /* MDEBUG */
			this->i_scope_histories.reserve(solution->curr_num_datapoints);
			this->i_is_higher_histories.reserve(solution->curr_num_datapoints);

			this->state = SEED_EXPERIMENT_STATE_TRAIN_FILTER;
			this->state_iter = 0;
			this->sub_state_iter = 0;
		} else {
			this->curr_filter->clean_fail();
			delete this->curr_filter;
			this->curr_filter = NULL;

			this->state_iter++;
			if (this->state_iter >= FIND_FILTER_ITER_LIMIT) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->state = SEED_EXPERIMENT_STATE_FIND_FILTER;
				create_filter();
				this->sub_state_iter = 0;
			}
		}
	}
}
