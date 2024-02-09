#include "branch_experiment.h"

using namespace std;

void BranchExperiment::verify_existing_backprop(double target_val,
												RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (this->parent_pass_through_experiment == NULL) {
		if (!run_helper.exceeded_limit) {
			if (run_helper.max_depth > solution->max_depth) {
				solution->max_depth = run_helper.max_depth;
			}
		}
	}

	if (this->state == BRANCH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
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
		this->verify_existing_score_variance = sum_score_variance / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		this->o_target_val_histories.clear();

		this->state = BRANCH_EXPERIMENT_STATE_VERIFY_1ST;
		this->state_iter = 0;
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
		this->verify_existing_score_variance = sum_score_variance / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		this->o_target_val_histories.clear();

		this->state = BRANCH_EXPERIMENT_STATE_VERIFY_2ND;
		this->state_iter = 0;
	}
}
