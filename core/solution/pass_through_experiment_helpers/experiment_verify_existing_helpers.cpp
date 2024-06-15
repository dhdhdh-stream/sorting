#include "pass_through_experiment.h"

#include <iostream>

#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::experiment_verify_existing_activate(
		vector<ContextLayer>& context,
		PassThroughExperimentHistory* history) {
	history->predicted_scores.push_back(vector<double>(context.size(), 0.0));
	for (int l_index = 0; l_index < (int)context.size(); l_index++) {
		if (context[l_index].scope->eval_network != NULL) {
			context[l_index].scope_history->callback_experiment_history = history;
			context[l_index].scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			context[l_index].scope_history->callback_experiment_layers.push_back(l_index);
		}
	}
}

void PassThroughExperiment::experiment_verify_existing_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(context.back().scope_history);
	}
	for (int i_index = 0; i_index < (int)context.back().scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[context.back().scope_history->callback_experiment_indexes[i_index]]
			[context.back().scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void PassThroughExperiment::experiment_verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		double sum_score = 0.0;
		for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
			sum_score += history->predicted_scores[i_index][l_index];
		}
		double final_score = (sum_score / (int)history->predicted_scores[i_index].size() + target_val - solution->average_score) / 2.0;
		this->target_val_histories.push_back(final_score);
	}

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= VERIFY_NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / num_instances;

		this->target_val_histories.clear();

		this->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

		this->root_state = ROOT_EXPERIMENT_STATE_VERIFY;
		this->state_iter = 0;
	}
}
