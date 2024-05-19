// #include "new_info_experiment.h"

// #include "constants.h"
// #include "globals.h"
// #include "solution.h"
// #include "solution_helpers.h"

// using namespace std;

// void NewInfoExperiment::measure_existing_activate(
// 		NewInfoExperimentHistory* history) {
// 	history->instance_count++;
// }

// void NewInfoExperiment::measure_existing_backprop(
// 		double target_val,
// 		RunHelper& run_helper) {
// 	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

// 	this->o_target_val_histories.push_back(target_val);

// 	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

// 	if (this->parent_experiment == NULL) {
// 		if (!run_helper.exceeded_limit) {
// 			if (run_helper.max_depth > solution->max_depth) {
// 				solution->max_depth = run_helper.max_depth;
// 			}

// 			if (run_helper.num_actions > solution->max_num_actions) {
// 				solution->max_num_actions = run_helper.num_actions;
// 			}
// 		}
// 	}

// 	if ((int)this->o_target_val_histories.size() >= NUM_DATAPOINTS) {
// 		double sum_scores = 0.0;
// 		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 			sum_scores += this->o_target_val_histories[d_index];
// 		}
// 		this->existing_average_score = sum_scores / NUM_DATAPOINTS;

// 		double sum_score_variance = 0.0;
// 		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
// 		}
// 		this->existing_score_standard_deviation = sqrt(sum_score_variance / NUM_DATAPOINTS);
// 		if (this->existing_score_standard_deviation < MIN_STANDARD_DEVIATION) {
// 			this->existing_score_standard_deviation = MIN_STANDARD_DEVIATION;
// 		}

// 		this->o_target_val_histories.clear();

// 		this->info_score = 0.0;
// 		this->new_info_subscope = create_new_info_scope();

// 		this->i_scope_histories.reserve(NUM_DATAPOINTS);
// 		this->i_target_val_histories.reserve(NUM_DATAPOINTS);

// 		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
// 		this->num_instances_until_target = 1 + until_distribution(generator);

// 		this->state = NEW_INFO_EXPERIMENT_STATE_EXPLORE_INFO;
// 		this->state_iter = 0;
// 		this->sub_state_iter = 0;
// 	}
// }
