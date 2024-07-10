// #include "pass_through_experiment.h"

// #include "constants.h"
// #include "globals.h"
// #include "scope.h"
// #include "solution_set.h"

// using namespace std;

// void PassThroughExperiment::verify_existing_activate(
// 		PassThroughExperimentHistory* history) {
// 	history->instance_count++;
// }

// void PassThroughExperiment::verify_existing_backprop(
// 		double target_val,
// 		RunHelper& run_helper) {
// 	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

// 	for (int i_index = 0; i_index < history->instance_count; i_index++) {
// 		double final_score = (target_val - solution_set->average_score) / history->instance_count;
// 		this->target_val_histories.push_back(final_score);
// 	}

// 	this->state_iter++;
// 	if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS
// 			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
// 		int num_instances = (int)this->target_val_histories.size();

// 		double sum_scores = 0.0;
// 		for (int d_index = 0; d_index < num_instances; d_index++) {
// 			sum_scores += this->target_val_histories[d_index];
// 		}
// 		this->existing_average_score = sum_scores / num_instances;

// 		this->target_val_histories.clear();

// 		this->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

// 		this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_NEW;
// 		this->state_iter = 0;
// 	}
// }
