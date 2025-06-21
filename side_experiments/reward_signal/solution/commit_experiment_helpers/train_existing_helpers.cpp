// #include "commit_experiment.h"

// #include <iostream>

// #include "constants.h"
// #include "globals.h"
// #include "scope.h"
// #include "solution_helpers.h"
// #include "solution_wrapper.h"

// using namespace std;

// #if defined(MDEBUG) && MDEBUG
// const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
// #else
// const int TRAIN_EXISTING_NUM_DATAPOINTS = 100;
// #endif /* MDEBUG */

// void CommitExperiment::train_existing_check_activate(
// 		SolutionWrapper* wrapper,
// 		CommitExperimentHistory* history) {
// 	history->instance_count++;

// 	this->scope_histories.push_back(new ScopeHistory(wrapper->scope_histories.back()));
// }

// void CommitExperiment::train_existing_backprop(
// 		double target_val,
// 		CommitExperimentHistory* history) {
// 	for (int i_index = 0; i_index < history->instance_count; i_index++) {
// 		this->i_target_val_histories.push_back(target_val);
// 	}

// 	this->sum_num_instances += history->instance_count;

// 	this->state_iter++;
// 	if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
// 		this->average_instances_per_run = (double)this->sum_num_instances / (double)this->state_iter;
// 		if (this->average_instances_per_run < 1.0) {
// 			this->average_instances_per_run = 1.0;
// 		}

// 		double average_score;
// 		vector<Input> factor_inputs;
// 		vector<double> factor_input_averages;
// 		vector<double> factor_input_standard_deviations;
// 		vector<double> factor_weights;
// 		double select_percentage;
// 		bool is_success = train_helper(this->scope_histories,
// 									   this->i_target_val_histories,
// 									   average_score,
// 									   factor_inputs,
// 									   factor_input_averages,
// 									   factor_input_standard_deviations,
// 									   factor_weights,
// 									   this->node_context,
// 									   this,
// 									   select_percentage);

// 		for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
// 			delete this->scope_histories[h_index];
// 		}
// 		this->scope_histories.clear();
// 		this->i_target_val_histories.clear();

// 		if (is_success) {
// 			this->existing_average_score = average_score;
// 			this->existing_inputs = factor_inputs;
// 			this->existing_input_averages = factor_input_averages;
// 			this->existing_input_standard_deviations = factor_input_standard_deviations;
// 			this->existing_weights = factor_weights;

// 			this->best_surprise = 0.0;

// 			uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
// 			this->num_instances_until_target = 1 + until_distribution(generator);

// 			this->state = COMMIT_EXPERIMENT_STATE_EXPLORE;
// 			this->state_iter = 0;
// 		} else {
// 			this->result = EXPERIMENT_RESULT_FAIL;
// 		}
// 	}
// }
