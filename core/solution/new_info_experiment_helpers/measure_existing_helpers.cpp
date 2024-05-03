#include "new_info_experiment.h"

using namespace std;

void NewInfoExperiment::measure_existing_activate(
		NewInfoExperimentHistory* history) {
	history->instance_count++;
}

void NewInfoExperiment::measure_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

	this->o_target_val_histories.push_back(target_val);

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if ((int)this->o_target_val_histories.size() >= NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / NUM_DATAPOINTS;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_standard_deviation = sqrt(sum_score_variance / NUM_DATAPOINTS);
		if (this->existing_score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->existing_score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->o_target_val_histories.clear();

		uniform_int_distribution<int> best_distribution(0, 1);
		if (best_distribution(generator) == 0) {
			this->explore_type = EXPLORE_TYPE_BEST;

			this->best_sequence_surprise = 0.0;
		} else {
			this->explore_type = EXPLORE_TYPE_GOOD;
		}

		this->state = NEW_INFO_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
