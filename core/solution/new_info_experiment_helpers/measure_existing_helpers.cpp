#include "new_info_experiment.h"

#include <iostream>

#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void NewInfoExperiment::measure_existing_activate(
		vector<ContextLayer>& context,
		NewInfoExperimentHistory* history) {
	history->instance_count++;

	history->predicted_scores.push_back(vector<double>(context.size()-1, 0.0));
	for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
		Scope* scope = (Scope*)context[l_index].scope;
		ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;
		if (scope->eval_network != NULL) {
			scope_history->callback_experiment_history = history;
			scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			scope_history->callback_experiment_layers.push_back(l_index);
		}
	}
}

void NewInfoExperiment::measure_existing_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

	ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(scope_history);
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void NewInfoExperiment::measure_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		double sum_score = 0.0;
		for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
			sum_score += history->predicted_scores[i_index][l_index];
		}
		sum_score += target_val - solution->average_score;
		double final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);
		this->target_val_histories.push_back(final_score);
	}

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / num_instances;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_score_variance += (this->target_val_histories[d_index] - this->existing_average_score) * (this->target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_standard_deviation = sqrt(sum_score_variance / num_instances);
		if (this->existing_score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->existing_score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->target_val_histories.clear();

		this->info_score = 0.0;
		this->new_info_scope = create_new_info_scope();

		this->scope_histories.reserve(NUM_DATAPOINTS);
		this->target_val_histories.reserve(NUM_DATAPOINTS);

		this->state = NEW_INFO_EXPERIMENT_STATE_EXPLORE_INFO;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
