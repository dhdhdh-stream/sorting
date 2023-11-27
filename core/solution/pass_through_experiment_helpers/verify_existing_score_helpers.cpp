#include "pass_through_experiment.h"

#include <iostream>

#include "globals.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::verify_existing_score_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (this->parent_pass_through_experiment == NULL) {
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
	}

	if ((int)this->o_target_val_histories.size() >= 2 * solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < 2 * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / (2 * solution->curr_num_datapoints);

		cout << "PassThrough" << endl;
		cout << "verify" << endl;
		cout << "this->existing_average_score: " << this->existing_average_score << endl;
		cout << endl;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < 2 * solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_variance = sum_score_variance / (2 * solution->curr_num_datapoints);

		this->o_target_val_histories.clear();

		this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_NEW_SCORE;
		this->state_iter = 0;
	}
}
