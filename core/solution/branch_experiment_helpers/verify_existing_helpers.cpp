#include "branch_experiment.h"

#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void BranchExperiment::verify_existing_activate(
		vector<ContextLayer>& context,
		BranchExperimentHistory* history) {
	switch (this->score_type) {
	case SCORE_TYPE_LOCAL:
		{
			double starting_predicted_score = calc_score(context.back().scope_history);
			history->starting_predicted_scores.push_back(vector<double>(starting_predicted_score));
			history->ending_predicted_scores.push_back(vector<double>(1));
			context.back().scope_history->callback_experiment_history = history;
			context.back().scope_history->callback_experiment_indexes.push_back(
				(int)history->starting_predicted_scores.size()-1);
			context.back().scope_history->callback_experiment_layers.push_back(0);
		}
		break;
	case SCORE_TYPE_ALL:
		history->starting_predicted_scores.push_back(vector<double>(context.size()));
		history->ending_predicted_scores.push_back(vector<double>(context.size()));
		for (int l_index = 0; l_index < (int)context.size(); l_index++) {
			double starting_predicted_score = calc_score(
				context[l_index].scope_history);
			history->starting_predicted_scores.back()[l_index] = starting_predicted_score;

			context[l_index].scope_history->callback_experiment_history = history;
			context[l_index].scope_history->callback_experiment_indexes.push_back(
				(int)history->starting_predicted_scores.size()-1);
			context[l_index].scope_history->callback_experiment_layers.push_back(l_index);
		}
		break;
	case SCORE_TYPE_FINAL:
		history->starting_predicted_scores.push_back(vector<double>());
		history->ending_predicted_scores.push_back(vector<double>());
		break;
	}
}

void BranchExperiment::verify_existing_back_activate(
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
		history->ending_predicted_scores[context.back().scope_history->callback_experiment_indexes[i_index]]
			[context.back().scope_history->callback_experiment_layers[i_index]] = ending_predicted_score;
	}
}

void BranchExperiment::verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	switch (this->score_type) {
	case SCORE_TYPE_LOCAL:
		for (int i_index = 0; i_index < (int)history->starting_predicted_scores.size(); i_index++) {
			this->target_val_histories.push_back(history->ending_predicted_scores[i_index][0]
				- history->starting_predicted_scores[i_index][0]);
		}
		break;
	case SCORE_TYPE_ALL:
		{
			double final_score = target_val - solution->average_score;
			for (int i_index = 0; i_index < (int)history->starting_predicted_scores.size(); i_index++) {
				double sum_score = 0.0;
				for (int l_index = 0; l_index < (int)history->starting_predicted_scores[i_index].size(); l_index++) {
					sum_score += history->ending_predicted_scores[i_index][l_index]
						- history->starting_predicted_scores[i_index][l_index];
				}
				sum_score += final_score;
				this->target_val_histories.push_back(sum_score);
			}
		}
		break;
	case SCORE_TYPE_FINAL:
		{
			double final_score = target_val - solution->average_score;
			for (int i_index = 0; i_index < (int)history->starting_predicted_scores.size(); i_index++) {
				this->target_val_histories.push_back(final_score);
			}
		}
		break;
	}

	this->state_iter++;
	bool is_done = false;
	if (this->score_type == SCORE_TYPE_FINAL) {
		if (this->state_iter >= FINAL_MIN_NUM_RUNS
				&& (int)this->target_val_histories.size() >= VERIFY_NUM_DATAPOINTS) {
			is_done = true;
		}
	} else {
		if ((int)this->target_val_histories.size() >= VERIFY_NUM_DATAPOINTS) {
			is_done = true;
		}
	}
	if (is_done) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / VERIFY_NUM_DATAPOINTS;

		this->target_val_histories.clear();

		this->combined_score = 0.0;

		this->state = BRANCH_EXPERIMENT_STATE_VERIFY;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
