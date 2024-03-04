#include "pass_through_experiment.h"

#include "constants.h"
#include "globals.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::measure_existing_activate(
		PassThroughExperimentHistory* history) {
	history->instance_count++;
}

void PassThroughExperiment::measure_existing_backprop(
		double target_val,
		RunHelper& run_helper,
		PassThroughExperimentHistory* history) {
	this->o_target_val_histories.push_back(target_val);

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (this->parent_experiment == NULL) {
		if (!run_helper.exceeded_limit) {
			if (run_helper.max_depth > solution->max_depth) {
				solution->max_depth = run_helper.max_depth;
			}
		}
	}

	if ((int)this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / solution->curr_num_datapoints;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_standard_deviation = sqrt(sum_score_variance / solution->curr_num_datapoints);
		if (this->existing_score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->existing_score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->o_target_val_histories.clear();

		this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPLORE_CREATE;
		this->state_iter = 0;
	}
}
