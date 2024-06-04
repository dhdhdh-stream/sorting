#include "branch_experiment.h"

#include <iostream>

#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void BranchExperiment::experiment_verify_existing_activate(
		vector<ContextLayer>& context,
		BranchExperimentHistory* history) {
	history->starting_predicted_scores.push_back(vector<double>(context.size(), 0.0));
	history->normalized_scores.push_back(vector<double>(context.size(), 0.0));
	for (int l_index = 0; l_index < (int)context.size(); l_index++) {
		if (context[l_index].scope->eval_network != NULL) {
			double starting_predicted_score = calc_score(context[l_index].scope_history);
			history->starting_predicted_scores.back()[l_index] = starting_predicted_score;

			context[l_index].scope_history->callback_experiment_history = history;
			context[l_index].scope_history->callback_experiment_indexes.push_back(
				(int)history->starting_predicted_scores.size()-1);
			context[l_index].scope_history->callback_experiment_layers.push_back(l_index);
		}
	}
}

void BranchExperiment::experiment_verify_existing_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	double ending_predicted_score;
	if (run_helper.num_actions > solution->num_actions_limit) {
		ending_predicted_score = -1.0;
	} else {
		ending_predicted_score = calc_score(context.back().scope_history);
	}
	for (int i_index = 0; i_index < (int)context.back().scope_history->callback_experiment_indexes.size(); i_index++) {
		double predicted_score = ending_predicted_score
			- history->starting_predicted_scores[context.back().scope_history->callback_experiment_indexes[i_index]]
				[context.back().scope_history->callback_experiment_layers[i_index]];
		history->normalized_scores[context.back().scope_history->callback_experiment_indexes[i_index]]
			[context.back().scope_history->callback_experiment_layers[i_index]] = predicted_score / context.back().scope->eval_score_standard_deviation;
	}
}

void BranchExperiment::experiment_verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	double final_normalized_score = (target_val - solution->average_score) / solution->score_standard_deviation;
	for (int i_index = 0; i_index < (int)history->starting_predicted_scores.size(); i_index++) {
		double sum_score = 0.0;
		for (int l_index = 0; l_index < (int)history->starting_predicted_scores[i_index].size(); l_index++) {
			sum_score += history->normalized_scores[i_index][l_index];
		}
		double final_score = sum_score / (int)history->starting_predicted_scores.size() + final_normalized_score;
		this->target_val_histories.push_back(final_score);
	}

	if ((int)this->target_val_histories.size() >= VERIFY_NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / VERIFY_NUM_DATAPOINTS;

		this->target_val_histories.clear();

		this->combined_score = 0.0;

		this->root_state = ROOT_EXPERIMENT_STATE_VERIFY;
		this->state_iter = 0;
	}
}
