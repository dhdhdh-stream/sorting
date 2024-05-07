#include "info_pass_through_experiment.h"

#include "constants.h"
#include "globals.h"
#include "new_action_tracker.h"
#include "solution.h"

using namespace std;

void InfoPassThroughExperiment::verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (!run_helper.exceeded_limit) {
		if (run_helper.max_depth > solution->max_depth) {
			solution->max_depth = run_helper.max_depth;
		}

		if (run_helper.num_actions > solution->max_num_actions) {
			solution->max_num_actions = run_helper.num_actions;
		}
	}

	if (run_helper.new_action_history != NULL) {
		for (int n_index = 0; n_index < (int)run_helper.new_action_history->existing_path_taken.size(); n_index++) {
			NewActionNodeTracker* node_tracker = solution->new_action_tracker->node_trackers[
				run_helper.new_action_history->existing_path_taken[n_index]];
			node_tracker->existing_score += target_val;
			node_tracker->existing_count++;
		}
		for (int n_index = 0; n_index < (int)run_helper.new_action_history->new_path_taken.size(); n_index++) {
			NewActionNodeTracker* node_tracker = solution->new_action_tracker->node_trackers[
				run_helper.new_action_history->new_path_taken[n_index]];
			node_tracker->new_score += target_val;
			node_tracker->new_count++;
		}
	}

	if (this->state == INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
			&& (int)this->o_target_val_histories.size() >= VERIFY_1ST_NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / VERIFY_1ST_NUM_DATAPOINTS;

		this->o_target_val_histories.clear();

		this->o_target_val_histories.reserve(VERIFY_1ST_NUM_DATAPOINTS);

		this->state = INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST;
		this->state_iter = 0;
	} else if ((int)this->o_target_val_histories.size() >= VERIFY_2ND_NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / VERIFY_2ND_NUM_DATAPOINTS;

		this->o_target_val_histories.clear();

		this->o_target_val_histories.reserve(VERIFY_2ND_NUM_DATAPOINTS);

		this->state = INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND;
		this->state_iter = 0;
	}
}
