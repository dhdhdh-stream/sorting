#include "experiment.h"

#include "constants.h"
#include "globals.h"
#include "solution.h"

using namespace std;

void Experiment::experiment_verify_existing_backprop(
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

		if (run_helper.num_actions > solution->max_num_actions) {
			solution->max_num_actions = run_helper.num_actions;
		}
	}

	if (this->state == EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING
			&& (int)this->o_target_val_histories.size() >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->verify_existing_average_score = sum_scores / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->verify_existing_average_score) * (this->o_target_val_histories[d_index] - this->verify_existing_average_score);
		}
		this->verify_existing_score_standard_deviation = sqrt(sum_score_variance / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));
		if (this->verify_existing_score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->verify_existing_score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->o_target_val_histories.clear();

		this->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		this->state = EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST;
		/**
		 * - leave this->experiment_iter unchanged
		 */
	} else if ((int)this->o_target_val_histories.size() >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->verify_existing_average_score = sum_scores / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->verify_existing_average_score) * (this->o_target_val_histories[d_index] - this->verify_existing_average_score);
		}
		this->verify_existing_score_standard_deviation = sqrt(sum_score_variance / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));
		if (this->verify_existing_score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->verify_existing_score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->o_target_val_histories.clear();

		this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		this->state = EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND;
		/**
		 * - leave this->experiment_iter unchanged
		 */
	}
}
